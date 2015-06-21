#!/usr/bin/env python

import md5
import sys
import os
import argparse
import multiprocessing
from contextlib import contextmanager
import logging
import subprocess as sub
import shutil

def add_coloring_to_emit_ansi(fn):
    # add methods we need to the class
    def new(*args):
        levelno = args[0].levelno
        if(levelno>=50):
            color = '\x1b[31m' # red
        elif(levelno>=40):
            color = '\x1b[31m' # red
        elif(levelno>=30):
            color = '\x1b[33m' # yellow
        elif(levelno>=20):
            color = '\x1b[32m' # green
        elif(levelno>=10):
            color = '\x1b[35m' # pink
        else:
            color = '\x1b[0m' # normal
        args[0].msg = color + args[0].msg +  '\x1b[0m'  # normal
        return fn(*args)
    return new 

@contextmanager
def pushd(newDir):
    previousDir = os.getcwd()
    os.chdir(newDir)
    yield
    os.chdir(previousDir)


def opt_to_str(o):
    return o.lstrip("-").replace("-","_")

def str_to_opt(s):
    return "--" + s.replace("_","-")

THIS_FILE = os.path.realpath(__file__)

class DnnSim(object):
    JOBS = multiprocessing.cpu_count()
    EPOCHS = 1
    RUNS_DIR = os.getenv("RUNS_DIR", os.path.expanduser("~/dnn/runs"))
    CONST_JSON = os.path.join(os.path.dirname(THIS_FILE), "../", "const.json")
    DNN_SIM_BIN = os.path.join(os.path.dirname(THIS_FILE), "../build/bin", "dnn_sim")
    INSP_SCRIPT = os.path.join(os.path.dirname(THIS_FILE), "../r_package/r_scripts", "insp.R")

    def __init__(self, **kwargs):
        self.current_epoch = 1

        self.old_dir = self.dget(kwargs, "old_dir", False)
        self.const = self.dget(kwargs, "const", self.CONST_JSON)
        self.runs_dir = self.dget(kwargs, "runs_dir", self.RUNS_DIR)
        self.inspection = self.dget(kwargs, "inspection", True)
        self.working_dir = self.dget(kwargs, "working_dir", self.get_wd())

        logFormatter = logging.Formatter("%(asctime)s [%(levelname)s]  %(message)-100s")
        rootLogger = logging.getLogger()
        rootLogger.setLevel(logging.DEBUG)

        consoleHandler = logging.StreamHandler(sys.stdout)
        consoleHandler.emit = add_coloring_to_emit_ansi(consoleHandler.emit)
        consoleHandler.setFormatter(logFormatter)
        rootLogger.addHandler(consoleHandler)

        if os.path.exists(self.working_dir):
            self.continue_in_wd()
        else:
            os.mkdir(self.working_dir)

        
        fileHandler = logging.FileHandler("{0}/{1}".format(self.working_dir, "run_sim.log"), mode='w')
        fileHandler.setFormatter(logFormatter)
        rootLogger.addHandler(fileHandler)



        self.dnn_sim_bin = self.dget(kwargs, "dnn_sim_bin", self.DNN_SIM_BIN)
        self.add_options = self.dget(kwargs, "add_options", {})
        self.stat = self.dget(kwargs, "stat", False)
        self.epochs = self.dget(kwargs, "epochs", 1)
        self.jobs = self.dget(kwargs, "jobs", multiprocessing.cpu_count())
        self.insp_script = self.dget(kwargs, "insp_script", self.INSP_SCRIPT)
        self.T_max = self.dget(kwargs, "T_max", None)
 
        for k, v in self.add_options.items():
            if os.path.exists(v):
                shutil.copy(v, os.path.join(self.working_dir, k) + ".pb")
        shutil.copy(self.const, self.working_dir)



    staticmethod
    def dget(self, d, n, default):
        if d.get(n) is None:
            return default
        else:
            return d[n]

    def get_opt(self, n):
        return str_to_opt(n), str(self.__dict__[n])

    def get_fname(self, f, ep=None):
        return os.path.join(self.working_dir, "{}_{}".format(ep if not ep is None else self.current_epoch, f)) 

    def construct_cmd(self):
        cmd = [
              self.dnn_sim_bin
        ]
        cmd += list(self.get_opt("const"))
        cmd += list(self.get_opt("jobs")) 
        cmd += [
            "--save", self.get_fname("model.pb"),
            "--output", self.get_fname("spikes.pb"),
        ]
        if self.stat:
            cmd += [
                "--stat", self.get_fname("stat.pb")
            ]
        prev_model = self.get_fname("model.pb", self.current_epoch-1)
        if os.path.exists(prev_model):
            cmd += [
                "--load", prev_model
            ]
        if not self.T_max is None:
            cmd += list(self.get_opt("T_max"))

        for k, v in self.add_options.items():
            cmd += [str_to_opt(k), v]

        return { "cmd" : cmd }
    
    def construct_inspect_cmd(self):
        env = {
            "T1" : "2000",
            "COPY_PICS" : "yes",
            "EP" : str(self.current_epoch),
            "OPEN_PIC" : "no",
            "SP_PIX0" : "{}".format(1024*2),
        }
        cmd = [
              self.insp_script
        ]
        return { "cmd" : cmd, "env" : env }

    staticmethod
    def run_proc(self, **args):
        env = os.environ
        add_env = args.get("env", {})
        env.update(add_env)
        cmd = args.get("cmd", [])
        if len(cmd) == 0:
            raise Exception("Got null command")

        if len(add_env) > 0:
            logging.info("env: {}".format(add_env))
        logging.info(" ".join(cmd))
        p = sub.Popen(cmd, stdout=sub.PIPE, stderr=sub.PIPE, env=env)
        stdout, stderr = p.communicate()
        if p.returncode != 0:
            logging.error("process failed:")
            if stdout:
                logging.error("\n\t"+stdout)
            if stderr:
                logging.error("\n\t"+stderr)
            sys.exit(-1)

    def run(self):
        for self.current_epoch in xrange(self.current_epoch, self.epochs+1):
            logging.info("Running epoch {}:".format(self.current_epoch))
            self.run_proc(**self.construct_cmd())
            if self.inspection:
                logging.info("inspecting ... ")
                with pushd(self.working_dir):
                    self.run_proc(**self.construct_inspect_cmd())

    def continue_in_wd(self):
        max_ep = 0
        for f in os.listdir(self.working_dir):
            f_spl = f.split("_")
            if len(f_spl) > 1 and "model" in f:
                max_ep = max(max_ep, int(f_spl[0]))
        if max_ep>0:
            while True:
                ans = raw_input("%s already exists and %s epochs was done here. Continue learning? (y/n): " % (os.path.basename(self.working_dir), max_ep))
                if ans in ["Y","y"]:
                    self.current_epoch = max_ep + 1
                    break
                elif ans in ["N", "n"]:
                    logging.info("Cleaning %s ... " % self.working_dir)
                    for f in os.listdir(self.working_dir):
                        os.remove(os.path.join(self.working_dir, f))
                    break                        
                else:
                    logging.warning("incomprehensible answer")


    def get_wd(self):
        const_hex = md5.new(open(self.const).read()).hexdigest()
        i = 0

        while i<1000:
            self.working_dir = os.path.join(self.runs_dir, const_hex + "_%04d" % i)
            if not os.path.exists(self.working_dir):
                break
            if self.old_dir:
                break
            i+=1
    
        return self.working_dir




    



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Tool for simulating dnn')

    parser.add_argument('-e', 
                        '--epochs', 
                        required=False,
                        help='Number of epochs to run', default=DnnSim.EPOCHS,type=int)
    parser.add_argument('-j', 
                        '--jobs', 
                        required=False,
                        help='Number of parallell jobs (default: %(default)s)', default=DnnSim.JOBS, type=int)
    parser.add_argument('-T', 
                        '--T-max', 
                        required=False,
                        help='Run only specific amount of simulation time (ms)')
    parser.add_argument('-o', 
                        '--old-dir', 
                        action='store_true',
                        help='Do not create new dir for that simulation')
    parser.add_argument('-s', 
                        '--stat',
                        action='store_true',
                        help='Save statistics')
    parser.add_argument('-ni', 
                        '--no-insp',
                        action='store_true',
                        help='No inspection after every epoch')
    parser.add_argument('-r', 
                        '--runs-dir', 
                        required=False,
                        help='Runs dir (default: %(default)s)', default=DnnSim.RUNS_DIR)
    parser.add_argument('-w', 
                        '--working-dir',
                        required=False,
                        help='Working dir (default: %%runs_dir%%/%%md5_of_const%%_%%number_of_experiment%%)')
    parser.add_argument('-c', 
                        '--const', 
                        required=False,
                        help='Path to const.json file (default: $SCRIPT_DIR/../%s)' % DnnSim.CONST_JSON)
    parser.add_argument('--dnn-sim-bin', 
                        required=False,
                        help='Path to snn sim bin (default: $SCRIPT_DIR/../build/bin/%s)' % DnnSim.DNN_SIM_BIN)
    args, other = parser.parse_known_args(sys.argv[1:])    
    if len(sys.argv) == 1:
        parser.print_help()
        sys.exit(1)
    args = {
        "working_dir" : args.working_dir,
        "T_max" : args.T_max,
        "stat" : args.stat,
        "runs_dir" : args.runs_dir,
        "const" : args.const,
        "dnn_sim_bin" : args.dnn_sim_bin,
        "add_options" : {},
        "epochs" : args.epochs,
        "jobs" : args.jobs,
        "old_dir" : args.old_dir,
        "inspection" : not args.no_insp,
    }
    if len(other) % 2 != 0:
        raise Exception("Got not paired add options: {}".format(" ".join(other)))
    for i in range(0, len(other), 2):
        args['add_options'].update( { opt_to_str(other[i]) : other[i+1] })


    s = DnnSim(**args)
    s.run()
