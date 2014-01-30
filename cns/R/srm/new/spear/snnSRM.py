#!/usr/bin/env python

import subprocess

codedir = '/home/alexeyche/my/git/alexeyche-junk/cns/R/srm/new'
cmd = "/usr/local/bin/R --slave --args %(run_name)s %(alpha)s %(beta)s %(lr)s %(llh_depr)s %(refr)s < %(codedir)s/main_ucr_opt.R 2> /dev/null"

def run_snn(d):
    p = subprocess.Popen(cmd % d, stdout=subprocess.PIPE, shell=True) 
    sin, _ = p.communicate()
    return float(sin.strip())

def main(job_id, params):
    l = run_snn(  
                  { 
                      'codedir' : codedir, 
                      'run_name' : 'run%s' % job_id,
                      'alpha' : params['alpha'][0],
                      'beta' : params['beta'][0],
                      'lr' : params['lr'][0],
                      'llh_depr' : params['llh_depr'][0],
                      'refr' : params['refr'][0]
                  }
               )
    return l

if __name__ == '__main__':
    print main(1, { 'alpha' : [1], 'beta' : [1], 'lr' : [0.01], 'llh_depr' : [0.04], 'refr' : ['low'] })
 
