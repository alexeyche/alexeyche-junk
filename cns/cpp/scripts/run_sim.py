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


RE_NUMBERED_FILE = re.compile("^([0-9]+)")
def get_last_ep(wd):
    l = []
    for f in os.listdir(wd):
        m = RE_NUMBERED_FILE.match(f)        
        if m:
            l.append(int(m.group(1)))
    return max(l)
    
class SnnProc(object):
    class EvalMethods(object):
        CLUSTERING = "clustering"
        NN_NMI = "nn_nmi"

    def __init__(self, wd, args):
        self.wd = wd
        self.args = args

    def proc_p_stat(self, p_stat, spikes, test_p_stat=None, test_spikes=None): # return path to json file with answer
        ep = get_last_ep(self.wd)
        proc_args = {}
        proc_args['--p-stat'] = p_stat
        proc_args['--spikes'] = spikes
        if test_spikes:
            proc_args['--test-spikes'] = test_spikes
        if test_p_stat:
            proc_args['--test-p-stat'] = test_p_stat
        proc_args['--jobs'] = self.args.jobs
        json_proc = os.path.join(self.wd, "%s_proc_output.json" % ep)
        proc_args['--output'] = json_proc
        proc_output = os.path.join(self.wd, "%s_snn_proc_output.log" % ep)
        runProcess([self.args.snn_proc_bin, "p_stat_dist"], proc_args, proc_output, verbose=self.args.verbose)
        if os.path.exists(proc_output) and os.stat(proc_output).st_size == 0:
            os.remove(proc_output)
        return json_proc
    
    def eval_dist_matrix(self, json_proc, eval_method):
        ep = get_last_ep(self.wd)
        j = json.load(open(json_proc))
        j['epoch'] = ep
        json.dump(j, open(json_proc, 'w'), indent=4)

        eval_output = os.path.join(self.wd, "%s_eval_pstat.log" % ep)
        eval_r_script = os.path.join(os.path.dirname(this_file), "eval_dist_matrix.R")
        with pushd(self.wd):
            runProcess(["Rscript", eval_r_script],  { "--method": eval_method, "--stat" : json_proc }, eval_output, verbose=self.args.verbose)
        
        stat = {} 
        stat['eval'] = float(open(eval_output).readlines()[-1].strip())
        stat['mean_rate'] = sum(j['rates'])/len(j['rates'])
        if self.args.verbose:
            for k in sorted(stat.keys()):
                print k, ":", stat[k],
            print
        return stat

    def eval_clustering(self, p_stat, spikes):
        json_proc = self.proc_p_stat(p_stat, spikes)
        return self.eval_dist_matrix(json_proc, SnnProc.EvalMethods.CLUSTERING)

    def eval_nn_nmi(self, p_stat, spikes, test_p_stat, test_spikes):
        json_proc = self.proc_p_stat(p_stat, spikes, test_p_stat, test_spikes)
        return self.eval_dist_matrix(json_proc, SnnProc.EvalMethods.NN_NMI)


class SnnSim(object):
    class Args(object):
        def setEvalCriterion(self, criterion_name):
            self.__dict__[criterion_name] = True

        input = None
        test_input = None
        spikes = None
        test_spikes = None
        epochs = 1
        eval_clustering_p_stat = False
        eval_nn_nmi = None
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

    def wd(self, s = None):
        if s:
            return os.path.join(self.workdir, s)
        else:
            return self.workdir

    def __init__(self, wd, args):
        self.args = args
        self.workdir = wd
        const = os.path.join(wd, os.path.basename(args.const))
        if not os.path.exists(const):
            shutil.copy(args.const, wd)

        input = None
        if args.input:
            input = args.input
        elif args.spikes:
            input = args.spikes

        self.common_sim_args = {
            '--jobs' : args.jobs,
            '--input' : input,
            '--constants' : const
        }            
        if args.T_max != 0:
            self.common_sim_args['--T-max'] = args.T_max
        self.last_run_epoch = None
        if args.tune_start_weights_to_target_rate:
            common.tuneStartWeights(args, wd, common_sim_args)

    def run_epoch(self, ep):
        sim_args = self.common_sim_args.copy()
        if self.args.eval_clustering_p_stat or self.args.eval_nn_nmi: 
            p_stat  = self.wd("%s_p_stat.pb" % ep)
            sim_args['--p-stat'] = p_stat
        
        sim_args['--save'] = self.wd("%s_model.pb" % ep)
        output_spikes = self.wd("%s_output_spikes.pb" % ep)
        sim_args['--output'] = output_spikes

        if self.args.stat:
            sim_args['--stat'] = self.wd("%s_stat.pb" % ep)
        elif self.args.p_stat and '--p-stat' not in sim_args:
            sim_args['--p-stat'] = self.wd("%s_p_stat.pb" % ep)

        if ep>1:
            model_load = self.wd("%s_model.pb" % str(ep-1))
            if not os.path.exists(model_load):
                raise Exception("Can't find model for previous epoch number %s" % str(ep-1))
            sim_args['--load'] = model_load

        if ep == 1 and self.args.collect_statistics:
            sim_args['--T-max'] = self.args.collect_statistics
            sim_args['--no-learning'] = True

        if self.args.input:
            if ep == 0:
                sim_args['--precalc'] = True
                sim_args['--output'] = self.wd("input_spikes.pb")
                del sim_args['--save']
            else:
                if not os.path.exists( self.wd("input_spikes.pb") ):
                    raise Exception("Can't find input_spikes.pb")
                sim_args['--input'] = self.wd("input_spikes.pb")
        elif ep == 0:
            return
        if self.args.epochs>1:
            print "Epoch %d" % ep
        runProcess(self.args.snn_sim_bin, sim_args, self.wd("%s_output.log" % ep), verbose = self.args.verbose)
        self.last_run_epoch = ep
        eval = {}
        if ep == 0:
            return eval
        if self.args.eval_clustering_p_stat:
            eval = SnnProc(self.wd(), self.args).eval_clustering(sim_args['--p-stat'], sim_args['--output'])
            os.remove(sim_args['--p-stat'])
        if self.args.eval_nn_nmi:
            if (self.args.test_input and self.args.test_spikes) and self.args.eval_nn_nmi:
                raise Exception("Need only test spikes or test time series. Not both")
            if self.args.test_input:
                sim_args['--input'] = self.args.test_input
            elif self.args.test_spikes:
                sim_args['--input'] = self.args.test_spikes
            else:
                raise Exception("Need test set for evaluation with NN NMI")
            test_output_spikes = self.wd("%s_test_output_spikes.pb" % ep)
            sim_args['--output'] = test_output_spikes
            test_p_stat = self.wd("%s_test_p_stat.pb" % ep)
            sim_args['--p-stat'] = test_p_stat
            sim_args['--no-learning'] = True
            
            runProcess(self.args.snn_sim_bin, sim_args, self.wd("%s_test_output.log" % ep), verbose = self.args.verbose)
            eval = SnnProc(self.wd(), self.args).eval_nn_nmi(p_stat, output_spikes, test_p_stat, test_output_spikes)
            
            os.remove(p_stat)
            os.remove(test_p_stat)
        return eval

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


    
    stat = {'eval' : None, 'mean_rate' : 0}
    sim = SnnSim(wd, args)
    for ep in xrange(start_ep, start_ep + args.epochs + 1):
        stat = sim.run_epoch(ep)
    return stat



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Tool for simulating snn')

    parser.add_argument('-i', 
                        '--input', 
                        required=False,
                        help='Input time series protobuf')
    parser.add_argument('-it', 
                        '--test-input', 
                        required=False,
                        help='Test input time series protobuf')
    parser.add_argument('-sp', 
                        '--spikes', 
                        required=False,
                        help='Train input labeled spikes list protobuf')
    parser.add_argument('-spt', 
                        '--test-spikes', 
                        required=False,
                        help='Test input labeled spikes list protobuf')
    parser.add_argument('-e', 
                        '--epochs', 
                        required=False,
                        help='Number of epochs to run', default=SnnSim.Args.epochs,type=int)
    parser.add_argument('-j', 
                        '--jobs', 
                        required=False,
                        help='Number of parallell jobs (default: %(default)s)', default=SnnSim.Args.jobs, type=int)
    parser.add_argument('-T', 
                        '--T-max', 
                        required=False,
                        help='Run only specific amount of simulation time (ms)', default=SnnSim.Args.T_max)
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
                        help='Runs dir (default: %(default)s)', default=SnnSim.Args.runs_dir)
    parser.add_argument('-w', 
                        '--working-dir',
                        required=False,
                        help='Working dir (default: %%runs_dir%%/%%md5_of_const%%_%%number_of_experiment%%)')
    parser.add_argument('-c', 
                        '--const', 
                        required=False,
                        help='Path to const.json file (default: $SCRIPT_DIR/../%s)' % CONST_JSON, default=SnnSim.Args.const)
    parser.add_argument('--snn-sim-bin', 
                        required=False,
                        help='Path to snn sim bin (default: $SCRIPT_DIR/../build/bin/%s)' % SNN_SIM, default=SnnSim.Args.snn_sim_bin)
    parser.add_argument('--snn-proc-bin', 
                        required=False,
                        help='Path to snn proc bin (default: $SCRIPT_DIR/../build/bin/%s)' % SNN_PROC, default=SnnSim.Args.snn_proc_bin)
    parser.add_argument('--eval-clustering-p-stat',
                        action='store_true',
                        help='Run evaluation of unsupervised classification with clustering of model intensities')
    parser.add_argument('--eval-nn-nmi',
                        action='store_true',
                        help='Run evaluation of net with nearest neighbourhood classificator 1 and NMI measure of confustion matrix (need test set of input)')
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
    sim_opts = SnnSim.Args()
    sim_opts.__dict__.update(args.__dict__)

    const = common.read_const(sim_opts.const)
    main(sim_opts)
