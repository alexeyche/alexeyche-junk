#!/usr/bin/env python

import argparse
import os
import sys
import md5
import shutil
import multiprocessing

from config import RUNS_DIR
from config import SNN_SIM
from config import CONST_JSON

import subprocess as sub
import time

def runSim(snn_sim_bin, args, log_stdout, verbose=False):
    cmd = [
            snn_sim_bin,
    ]            

    for k, v in [ (k, args[k]) for k in sorted(args) ]:
        if type(v) is bool:
            cmd += [k]
        else:
            cmd += [k, v]
    start_time = time.time()
    sp = sub.Popen(cmd, stdout=open(log_stdout, 'w'), stderr=sub.STDOUT, shell=False)
    if verbose:
        print 
        print " ".join(cmd)
        print 
    try:
        sp.communicate()
    except KeyboardInterrupt:
        print "Bye"
    end_time = time.time()
    if verbose:
        print "time run: %s sec" % str(end_time-start_time)
    if sp.returncode != 0:
        print "snn_sim failed: "
        with open(log_stdout) as lf:
            for l in lf:
                print l.strip()
        sys.exit(1)



def main(args):
    const_hex = md5.new(args.const).hexdigest()
    i=0
    wd = None
    while i<1000:
        new_wd = os.path.join(args.runs_dir, const_hex + "_%04d" % i)
        if not os.path.exists(new_wd):
            if not args.old_dir or not wd:
                wd = new_wd
            break
        wd = new_wd
        i+=1
    
    start_ep = 0
    if os.path.exists(wd):
        max_ep = -1
        for f in os.listdir(wd):
            f_spl = f.split("_")
            if len(f_spl) > 1 and "model" in f:
                max_ep = max(max_ep, int(f_spl[0]))
        if max_ep>0:
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

    shutil.copy(args.const, wd)
    const = os.path.join(wd, os.path.basename(args.const))

    def wd_file(s):
        return os.path.join(wd, s)
    input = None
    if args.input:
        input = args.input
    elif args.spikes:
        input = args.spikes

    common_sim_args = {
            '--jobs' : str(args.jobs),
            '--input' : input,
            '--constants' : const
    }            
    if args.T_max != 0:
        common_sim_args['--T-max'] = args.T_max

    for ep in xrange(start_ep, start_ep + args.epochs + 1):
        sim_args = common_sim_args.copy()
        
        sim_args['--save'] = wd_file("%s_model.pb" % ep)
        sim_args['--output'] = wd_file("%s_output_spikes.pb" % ep)
        if args.stat:
            sim_args['--stat'] = wd_file("%s_stat.pb" % ep)
        elif args.p_stat:
            sim_args['--p-stat'] = wd_file("%s_p_stat.pb" % ep)

        if ep>1:
            model_load = wd_file("%s_model.pb" % str(ep-1))
            if not os.path.exists(model_load):
                raise Exception("Can't find model for previous epoch number %s" % str(ep-1))
            sim_args['--load'] = model_load

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
        print "Epoch %d" % ep
        runSim(args.snn_sim_bin, sim_args, wd_file("%s_output.log" % ep), verbose = args.verbose)
    



if __name__ == '__main__':
    this_file = os.path.realpath(__file__)
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
                        help='Number of epochs to run', default=1,type=int)
    parser.add_argument('-j', 
                        '--jobs', 
                        required=False,
                        help='Number of parallell jobs (default: %(default)s)', default=multiprocessing.cpu_count())
    parser.add_argument('-T', 
                        '--T-max', 
                        required=False,
                        help='Run only specific amount of simulation time (ms)', default=0)
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
                        help='Runs dir (default: %(default)s)', default=RUNS_DIR)
    parser.add_argument('-c', 
                        '--const', 
                        required=False,
                        help='Path to const.json file (default: $SCRIPT_DIR/../%s)' % CONST_JSON, default=os.path.join(os.path.dirname(this_file), "../", CONST_JSON))
    parser.add_argument('--snn-sim-bin', 
                        required=False,
                        help='Path to snn sim bin (default: $SCRIPT_DIR/../build/bin/%s)' % SNN_SIM, default=os.path.join(os.path.dirname(this_file), "../build/bin", SNN_SIM))

    args = parser.parse_args(sys.argv[1:])    
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)

    if (not args.spikes and not args.input) or (args.spikes and args.input):
        raise Exception("Need input time series or input spikes")
    if args.stat and args.p_stat:
        raise Exception("Can't collect Full and Partial statistics. Choose one")
    main(args)
