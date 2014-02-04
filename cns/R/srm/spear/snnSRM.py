#!/usr/bin/env python

import subprocess
import tempfile
import shutil

codedir = '/home/alexeyche/my/git/alexeyche-junk/cns/R/srm'
#codedir = "/home/alexeyche/prog/alexeyche-junk/cns/R/srm"

script_name = 'main_ucr.R'
runs_dir = '/home/alexeyche/my/sim/runs'
#runs_dir = '/home/alexeyche/prog/sim/runs'

cmd = "Rscript %(codedir)s/%(script_name)s  \
            --const-file=%(const_file)s --run-name=%(run_name)s --runs-dir=%(runs_dir)s --no-verbose 2> %(runs_dir)s/%(run_name)s.err.log"

def run_snn(d):
    p = subprocess.Popen(cmd % d, stdout=subprocess.PIPE, shell=True) 
    sin, _ = p.communicate()
    return float(sin.strip())

def main(job_id, params):
    tmp = tempfile.mktemp(prefix='constants.R.')
    shutil.copy('constants.template', tmp)
    with open(tmp, 'r') as tmp_ptr:
        tmp_read = tmp_ptr.read()
    
    tmp_read = tmp_read % {  
                             'alpha' : params['alpha'][0],
                             'beta' : params['beta'][0],
                             'llh_depr_mode' : params['llh_depr_mode'][0],
                             'refr_mode' : params['refr_mode'][0],
                             'duration' : params['duration'][0],
                             'net_edge_prob' : params['net_edge_prob'][0], 
                             'inhib_frac' : params['inhib_frac'][0],
                             'net_neurons_for_input' : params['net_neurons_for_input'][0],
                             'afferent_per_neuron' : params['afferent_per_neuron'][0],
                             'weights_norm_type' : params['weights_norm_type'][0],
    }
    with open(tmp, 'w') as tmp_ptr:
        tmp_ptr.write(tmp_read)
    
    l = run_snn(  
                  { 
                      'codedir' : codedir, 
                      'script_name' : script_name,
                      'runs_dir' : runs_dir,
                      'run_name' : 'run%s' % job_id,
                      'const_file' : tmp
                  }
               )
    return l

if __name__ == '__main__':
    print main(1, { 'alpha' : [1], 'beta' : [1], 'lr' : [0.01], 'llh_depr_mode' : ['no'], 'refr_mode' : ['low'], 'duration' : [150], 'net_edge_prob' : [0.5], 'inhib_frac' : [0.2], 'net_neurons_for_input' : [25], 'afferent_per_neuron' : [10], 'weights_norm_type' : ['mult_local'] })
 
