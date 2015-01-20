#!/usr/bin/env python

import argparse
import os
import sys
import md5
import shutil
import multiprocessing
from contextlib import contextmanager
import json
import traceback
import collections
import numpy as np
import functools
import math
import re

from config import RUNS_DIR
from config import SNN_SIM
from config import SNN_PROC
from config import CONST_JSON

import subprocess as sub
import time

import common

this_file = os.path.realpath(__file__)


@contextmanager
def pushd(newDir):
    previousDir = os.getcwd()
    os.chdir(newDir)
    yield
    os.chdir(previousDir)


def runProcess(bin, args, log_stdout=None, verbose=False, json_stdout=False, do_not_wait=False):
    if type(bin) is list:
        cmd = list(bin)
    else:
        cmd = [
            bin,
        ]            
    for k, v in [ (k, args[k]) for k in sorted(args) ]:
        if type(v) is bool:
            cmd += [k]
        else:
            cmd += [k, str(v)]
    start_time = time.time()
    if log_stdout:
        sp = sub.Popen(cmd, stdout=open(log_stdout, 'w'), stderr=sub.STDOUT, shell=False)
    else:        
        sp = sub.Popen(cmd, stdout=sub.PIPE, stderr=sub.STDOUT, shell=False)
   
    if verbose:
        print 
        print " ".join(cmd)
        print 
    if do_not_wait:
        return sp
    try:
        stdout, stderr = sp.communicate()
    except KeyboardInterrupt:
        print "Bye"
    end_time = time.time()
    if verbose:
        print "time run: %s sec" % str(end_time-start_time)
    if sp.returncode != 0:
        if log_stdout:
            with open(log_stdout) as lf:
                for l in lf:
                    print l.strip()
        else:
            for l in stdout.split("\n"):
                print l.strip()
        raise Exception("Run failed")
    if not log_stdout and not json_stdout:
        return stdout
    if json_stdout:
        return json.loads(stdout, object_pairs_hook=collections.OrderedDict)


def evalPStat(args, wd, ep, p_stat_file, spikes, eval_method, stat):
    if args.verbose:
        print "Evaluation p stat, method %s..." % eval_method,
        print 
    proc_args = {}
    proc_args['--p-stat'] = p_stat_file
    proc_args['--spikes'] = spikes
    proc_args['--jobs'] = args.jobs
    json_proc = os.path.join(wd, "%s_proc_output.json" % ep)
    proc_args['--output'] = json_proc
    proc_output = os.path.join(wd, "%s_proc_stdout.log" % ep)
    runProcess([args.snn_proc_bin, "p_stat_dist"], proc_args, proc_output, verbose=args.verbose)
    if os.path.exists(proc_output) and os.stat(proc_output).st_size == 0:
        os.remove(proc_output)
    
    j = json.load(open(json_proc))
    j['epoch'] = ep 
    json.dump(j, open(json_proc, 'w'), indent=4)

    eval_output = os.path.join(wd, "%s_eval_pstat.log" % ep)
    eval_r_script = os.path.join(os.path.dirname(this_file), "eval_dist_matrix.R")
    with pushd(wd):
        runProcess(["Rscript", eval_r_script],  { "--method": eval_method, "--stat" : json_proc }, eval_output, verbose=args.verbose)
    
    stat['eval'] = float(open(eval_output).read().strip())
    stat['mean_rate'] = sum(j['rates'])/len(j['rates'])
    if args.verbose:
        for k in sorted(stat.keys()):
            print k, ":", stat[k],
        print

def read_const(const_file):
    with open(const_file) as const_ptr:
        c_json = "\n".join([ l.split("//")[0].split("#")[0] for l in const_ptr.readlines() ])
    try:
        const = json.loads(c_json, object_pairs_hook=collections.OrderedDict)
    except:
        print "Error while reading %s:" % const_file
        print traceback.format_exc()
        sys.exit(1) 
    return const


DISTR_REG_EXP = "[A-Za-z]+\(([ 0-9.]+),([ 0-9.]+)\)"

def tuneStartWeights(args, wd, common_sim_args, weight_range=(2.5, 100, 500), T_max=2500, parallel_launches=4):
    const = read_const(common_sim_args['--constants'])
    d_re = re.compile(DISTR_REG_EXP)
    acc_w = 0.0
    conn_count = 0
    for c in const['sim_configuration']['conn_map']:
        for c_var in const['sim_configuration']['conn_map'][c]:
            m = d_re.match(c_var['weight_distr'])
            if m:
                acc_w += float(m.group(1))
                conn_count += 1
            else:
                raise Exception("Faild to parse distribution of weighgts: %s" % c_var['weight_distr'])
    start_weight = acc_w / conn_count

    sim_args = common_sim_args.copy()
    sim_args['--no-learning'] = True
    sim_args['--T-max'] = T_max
    sim_args['--constants'] = os.path.join(wd, "tune_const.json")
    
    def run_one_tune(args, wd, sim_args, const, w):
        for c in const['sim_configuration']['conn_map']:
            for i, c_var in enumerate(const['sim_configuration']['conn_map'][c]):
                c_var['weight_distr'] = "Norm(%s,0.5)" % w
                const['sim_configuration']['conn_map'][c][i] = c_var
        json.dump(const, open(sim_args['--constants'], 'w'), indent=4)

        procs = []
        for i in range(parallel_launches):
            sim_args_launch = sim_args.copy()
            sim_args_launch['--jobs'] = int(math.floor(float(sim_args_launch['--jobs'])/parallel_launches))
            if i == parallel_launches-1:
                 sim_args_launch['--jobs']  += sim_args['--jobs'] - sim_args_launch['--jobs'] * parallel_launches
            sim_args_launch['--output'] = os.path.join(wd, "tune_weights_output_spikes_%s.pb" % i)
            sp = runProcess(args.snn_sim_bin, sim_args_launch, os.path.join(wd, "tune_weights_output_%s.log" % i), verbose = args.verbose, do_not_wait=True)
            procs.append( (sim_args_launch, sp) )
        
        mean_acc = 0.0
        for sim_args_launch, sp in procs:
            try:
                stdout, stderr = sp.communicate()
            except KeyboardInterrupt:
                print "Bye"
            if sp.returncode != 0:
                print "Run failed: "
                for l in stdout.split("\n"):
                    print l.strip()
                sys.exit(1)
            proc_args = {
                '--spikes' : sim_args_launch['--output'],
                '--net-neurons' : sum([ conf['size'] for conf in  const['sim_configuration']['net_layers_conf'] ]),
            }
            json_output = runProcess([args.snn_proc_bin, "mean_net_rate"], proc_args, verbose=args.verbose, json_stdout=True)
            mean_acc += json_output['mean_rate']

        mean = mean_acc/parallel_launches            
        print "weight %s got %s mean rate" % (w, mean)
        return mean
    

    weights = np.linspace(weight_range[0], weight_range[1], weight_range[2])
    part_fun_tune = functools.partial(run_one_tune, args, wd, sim_args, const)

    pos = common.binary_search(weights, args.tune_start_weights_to_target_rate, 0.5, fun=part_fun_tune)
    if pos < 0:
        raise Exception("Failed to find good weight")

    new_const = read_const(sim_args['--constants'])
    for c in new_const['sim_configuration']['net_layers_conf']:
        #if 'learning_rule' in c['neuron_conf']:
        #    new_const['learning_rules'][ c['neuron_conf']['learning_rule'] ]['learning_rate'] *= weights[pos]/start_weight
        if 'weight_normalization' in c['neuron_conf']:
            if 'w_max' in new_const['weight_normalizations'][ c['neuron_conf']['weight_normalization'] ]:
                new_const['weight_normalizations'][ c['neuron_conf']['weight_normalization'] ]['w_max'] *= weights[pos]/start_weight
    json.dump(new_const, open(sim_args['--constants'], 'w'), indent=4)
    common_sim_args['--constants'] = sim_args['--constants']

def main(args):
    const_hex = md5.new(args.const).hexdigest()
    i=0
    wd = args.working_dir

    start_ep = 0
    if not wd:
        while i<1000:
            new_wd = os.path.join(args.runs_dir, const_hex + "_%04d" % i)
            if not os.path.exists(new_wd):
                if not args.old_dir or not wd:
                    wd = new_wd
                break
            wd = new_wd
            i+=1
        
        if os.path.exists(wd):
            max_ep = 0
            for f in os.listdir(wd):
                f_spl = f.split("_")
                if len(f_spl) > 1 and "model" in f:
                    max_ep = max(max_ep, int(f_spl[0]))
            if max_ep>=0:
                while True:
                    ans = raw_input("%s already exists and %s epochs was done here. Continue learning? (y/n): " % (os.path.basename(wd), max_ep))
                    if ans in ["Y","y"]:
                        start_ep = max_ep + 1
                        break
                    elif ans in ["N", "n"]:
                        print "Cleaning %s ... " % wd
                        for f in os.listdir(wd):
                            os.remove(os.path.join(wd, f))
                        break                        
                    else:
                        print "incomprehensible answer"
        else:
            os.mkdir(wd)

    const = os.path.join(wd, os.path.basename(args.const))
    if not os.path.exists(const):
        shutil.copy(args.const, wd)

    def wd_file(s):
        return os.path.join(wd, s)
    
    input = None
    if args.input:
        input = args.input
    elif args.spikes:
        input = args.spikes

    common_sim_args = {
        '--jobs' : args.jobs,
        '--input' : input,
        '--constants' : const
    }            
    if args.T_max != 0:
        common_sim_args['--T-max'] = args.T_max
    
    if args.tune_start_weights_to_target_rate:
        tuneStartWeights(args, wd, common_sim_args)

    stat = {'eval' : None, 'mean_rate' : 0}
    for ep in xrange(start_ep, start_ep + args.epochs + 1):
        sim_args = common_sim_args.copy()
        if args.eval_clustering_p_stat: 
            sim_args['--p-stat'] = wd_file("%s_p_stat.pb" % ep) 
        sim_args['--save'] = wd_file("%s_model.pb" % ep)
        sim_args['--output'] = wd_file("%s_output_spikes.pb" % ep)
        if args.stat:
            sim_args['--stat'] = wd_file("%s_stat.pb" % ep)
        elif args.p_stat and '--p-stat' not in sim_args:
            sim_args['--p-stat'] = wd_file("%s_p_stat.pb" % ep)

        if ep>1:
            model_load = wd_file("%s_model.pb" % str(ep-1))
            if not os.path.exists(model_load):
                raise Exception("Can't find model for previous epoch number %s" % str(ep-1))
            sim_args['--load'] = model_load

        if ep == 1 and args.collect_statistics:
            sim_args['--T-max'] = args.collect_statistics
            sim_args['--no-learning'] = True

        if args.input:
            if ep == 0:
                sim_args['--precalc'] = True
                sim_args['--output'] = wd_file("input_spikes.pb")
                del sim_args['--save']
            else:
                if not os.path.exists( wd_file("input_spikes.pb") ):
                    raise Exception("Can't find input_spikes.pb")
                sim_args['--input'] = wd_file("input_spikes.pb")
        elif ep == 0:
            continue
        if args.epochs>1:
            print "Epoch %d" % ep
        runProcess(args.snn_sim_bin, sim_args, wd_file("%s_output.log" % ep), verbose = args.verbose)
        if args.eval_clustering_p_stat and ep > 0: 
            evalPStat(args, wd, ep, sim_args['--p-stat'], sim_args['--output'], "clustering", stat)
            os.remove(sim_args['--p-stat'])
    return stat


class RunSimArgs(object):
    def setEvalCriterion(self, criterion_name):
        self.__dict__[criterion_name] = True

    input = None
    spikes = None
    epochs = 1
    eval_clustering_p_stat = False
    const = os.path.join(os.path.dirname(this_file), "../", CONST_JSON)
    snn_sim_bin = os.path.join(os.path.dirname(this_file), "../build/bin", SNN_SIM)
    snn_proc_bin = os.path.join(os.path.dirname(this_file), "../build/bin", SNN_PROC)
    working_dir = None
    jobs = multiprocessing.cpu_count()
    T_max = 0
    stat = None
    verbose = True
    old_dir = False
    p_stat = None
    runs_dir = RUNS_DIR
    collect_statistics = None
    tune_start_weights_to_target_rate = None

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Tool for simulating snn')

    parser.add_argument('-i', 
                        '--input', 
                        required=False,
                        help='Input time series protobuf')
    parser.add_argument('-sp', 
                        '--spikes', 
                        required=False,
                        help='Input labeled spikes list protobuf')
    parser.add_argument('-e', 
                        '--epochs', 
                        required=False,
                        help='Number of epochs to run', default=RunSimArgs.epochs,type=int)
    parser.add_argument('-j', 
                        '--jobs', 
                        required=False,
                        help='Number of parallell jobs (default: %(default)s)', default=RunSimArgs.jobs, type=int)
    parser.add_argument('-T', 
                        '--T-max', 
                        required=False,
                        help='Run only specific amount of simulation time (ms)', default=RunSimArgs.T_max)
    parser.add_argument('-o', 
                        '--old-dir', 
                        action='store_true',
                        help='Do not create new dir for that simulation')
    parser.add_argument('-s', 
                        '--stat',
                        action='store_true',
                        help='Save statistics')
    parser.add_argument('-ps', 
                        '--p-stat',
                        action='store_true',
                        help='Save probabilites of neurons')
    parser.add_argument('-v', 
                        '--verbose', 
                        action='store_true',
                        help='Verbose running snn_sim commands')
    parser.add_argument('-r', 
                        '--runs-dir', 
                        required=False,
                        help='Runs dir (default: %(default)s)', default=RunSimArgs.runs_dir)
    parser.add_argument('-w', 
                        '--working-dir',
                        required=False,
                        help='Working dir (default: %%runs_dir%%/%%md5_of_const%%_%%number_of_experiment%%)')
    parser.add_argument('-c', 
                        '--const', 
                        required=False,
                        help='Path to const.json file (default: $SCRIPT_DIR/../%s)' % CONST_JSON, default=RunSimArgs.const)
    parser.add_argument('--snn-sim-bin', 
                        required=False,
                        help='Path to snn sim bin (default: $SCRIPT_DIR/../build/bin/%s)' % SNN_SIM, default=RunSimArgs.snn_sim_bin)
    parser.add_argument('--snn-proc-bin', 
                        required=False,
                        help='Path to snn proc bin (default: $SCRIPT_DIR/../build/bin/%s)' % SNN_PROC, default=RunSimArgs.snn_proc_bin)
    parser.add_argument('--eval-clustering-p-stat',
                        action='store_true',
                        help='Run evaluation of unsupervised classification with clustering of model intensities')
    parser.add_argument('--collect-statistics',
                        required=False,
                        help='Run simulation in first epoch without learning to collect statistics')
    parser.add_argument('--tune-start-weights-to-target-rate',
                        required=False,
                        help='Run simulations to find optimal start weight for specific target rate', type=int)
    args = parser.parse_args(sys.argv[1:])    
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    if (not args.spikes and not args.input) or (args.spikes and args.input):
        raise Exception("Need input time series or input spikes")
    if args.stat and args.p_stat:
        raise Exception("Can't collect Full and Partial statistics. Choose one")
    sim_opts = RunSimArgs()
    sim_opts.__dict__.update(args.__dict__)

    const = read_const(sim_opts.const)
    main(sim_opts)
