#!/usr/bin/env python

import argparse
import os
import sys
import md5
import shutil

from config import RUNS_DIR
from config import SNN_SIM
from config import CONST_JSON

import subprocess as sub

def runSim(snn_sim_bin, args, wd, epoch):
    cmd = [
            snn_sim_bin,
    ]            
    for k, v in args.items():
        cmd.append( (k, v) )
    log_stdout = os.path.join("%s_output.log")
    sp = sub.Popen(cmd, stdout=open(log_stdout, 'w'), stderr=sub.STDOUT, shell=False)
    sp.communicate()
    if sp.returncode != 0:
        with open(log_stdout) as lf:
            for l in lf:
                print l.strip()
        sys.exit(1)

if __name__ == '__main__':
    this_file = os.path.realpath(__file__)
    parser = argparse.ArgumentParser(description='Tool for simulating snn')

    parser.add_argument('-i', 
                        '--input', 
                        required=True,
                        help='Input spikes list or time series protobuf')
    parser.add_argument('-e', 
                        '--epochs', 
                        required=False,
                        help='Number of epochs to run', default=1)
    parser.add_argument('-t', 
                        '--test-run', 
                        action='store_true',
                        help='Test run (for 1000 ms)')
    parser.add_argument('-o', 
                        '--old-dir', 
                        action='store_true',
                        help='Do not create new dir for that simulation')
    parser.add_argument('-c', 
                        '--const', 
                        required=False,
                        help='Path to const.json file (default: $SCRIPT_DIR/../%s)' % CONST_JSON, default=os.path.join(os.path.dirname(this_file), "../", CONST_JSON))
    parser.add_argument('--snn-sim-bin', 
                        required=False,
                        help='Path to snn sim bin (default: $SCRIPT_DIR/../build/bin/%s)' % SNN_SIM, default=os.path.join(os.path.dirname(this_file), "../build/bin", SNN_SIM))

    args = parser.parse_args(sys.argv[1:])    
    const_hex = md5.new(args.const).hexdigest()
    i=0
    while i<1000:
        new_wd = os.path.join(RUNS_DIR, const_hex + "_%04d" % i)
        if not os.path.exists(new_wd):
            if not args.old_dir:
                wd = new_wd
            break
        wd = new_wd
        i+=1
    
    start_ep = 1
    if os.path.exists(wd):
        max_ep = -1
        for f in os.listdir(wd):
            f_spl = f.split("_")
            if len(f_spl) > 1:
                max_ep = max(max_ep, int(f_spl[0]))
        if max_ep>0:
            while True:
                ans = raw_input("%s already exists and %s epochs was done here. Continue learning? (y/n):" % (os.path.basename(wd), max_ep))
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
                    
