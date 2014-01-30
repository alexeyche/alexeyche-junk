#!/usr/bin/env python

import subprocess

def run_snn(alpha, beta, lr, llh_depr, refr):
    cmd
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE) 
    sin, sout = p.communicate()
    return sin.strip()

def main(job_id, params):
    alpha = params['alpha'][0]
    beta = params['beta'][0]
    lr = params['lr'][0]
    llh_depr = params['llh_depr'][0]
    refr = params['refr'][0]
    run_snn(alpha, beta, lr, llh_depr, refr)
