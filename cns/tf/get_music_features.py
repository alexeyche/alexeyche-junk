#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Mon May  9 09:15:34 2016

@author: alexeyche
"""

import numpy as np
import librosa as lr
import argparse
from librosa.core.time_frequency import mel_frequencies
import subprocess as sub
import sys
import logging
import os


def run_proc(cmd, env = {}, print_root_log_on_fail=False, stdout=None, stderr=None):
    osenv = os.environ
    osenv.update(env)

    if len(env) > 0:
        logging.info("env: {}".format(env))
    logging.info(" ".join(cmd))
    p = sub.Popen(cmd, stdout=open(stdout, "w") if stdout else sub.PIPE, stderr = open(stderr, "w") if stderr else sub.PIPE, env=osenv)
    stdout_str, stderr_str = p.communicate()
    if p.returncode != 0:
        logging.error("process failed:")
        if stdout:
            stdout_str = open(stdout).read()
        if stderr:
            stderr_str = open(stderr).read()
        if stdout_str:
            logging.error("\n\t"+stdout_str)
        if stderr_str:
            logging.error("\n\t"+stderr_str)
        if print_root_log_on_fail:
            root_log = logging.getLogger()
            logs = [ h.stream.name for h in root_log.handlers if isinstance(h, logging.FileHandler) ]
            assert(len(logs)>0)
            print open(logs[0]).read()
        sys.exit(-1)
    return stdout_str


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

def setup_logging(logger, console_out=sys.stdout, level=logging.DEBUG):
    logFormatter = logging.Formatter("%(asctime)s [%(levelname)s]  %(message)-100s")
    logger.setLevel(level)
    if console_out:
        consoleHandler = logging.StreamHandler(console_out)
        consoleHandler.emit = add_coloring_to_emit_ansi(consoleHandler.emit)
        consoleHandler.setFormatter(logFormatter)
        logger.addHandler(consoleHandler)



setup_logging(logging.getLogger())


def main(input_file, ms_frame=10, n_mels=256, top_db=60, plot=False, fmin=100.0):
    input_file = os.path.realpath(os.path.expanduser(input_file))
    basef = input_file.rsplit(".",1)[0]
    logging.info("Processing {}".format(input_file))
    if input_file.endswith(".mp3"):
        logging.info("Got mp3 file, trying to convert")
        f = basef + ".wav"
        if not os.path.exists(f):
            run_proc(["mpg123", "-w", f, input_file])
        input_file = f
    elif input_file.endswith(".wav"):
        pass
    else:
        raise Exception("Need wav or mp3 file to process")

    y, sr = lr.load(input_file)
    hop = int(np.round(ms_frame*sr/1000.0))
    stft = lr.stft(y, hop_length=hop)
    D = lr.logamplitude(np.abs(stft)**2, ref_power=np.max, top_db=top_db)
    s = lr.feature.melspectrogram(S=D, sr=sr, n_mels=n_mels, fmin=fmin, fmax=sr/2.0)
    # mf = mel_frequencies(n_mels, fmin, sr/2.0)

    logging.info("Normalizing between 0 and 1")
    for ri in xrange(s.shape[0]):
        denom = np.max(s[ri,:]) - np.min(s[ri,:])
        if denom > 0:
            s[ri,:] = (s[ri,:] - np.min(s[ri,:]))/denom
        else:
            s[ri,:] = 0

    if plot:
        from matplotlib import pyplot as plt

        plt.figure(1)
        plt.subplot(2,1,1)
        lr.display.specshow(
            lr.logamplitude(
                    lr.stft(np.abs(y))**2,
                ref_power=np.max, top_db=top_db),
            sr, y_axis="mel")
        plt.subplot(2,1,2)
        lr.display.specshow(s, sr, hop_length=hop)
        plt.savefig(basef + ".png")

    np.save(basef+".data", s)

    return s

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Extracting mel spectrogram from file')
    parser.add_argument('file', nargs=1, help="Path to music file")
    parser.add_argument('--ms-frame', help='Ms in one frame', type=int, default=10)
    parser.add_argument('--n-mels', help='Number of mel components', type=int, default=256)
    parser.add_argument('--top-db', help='Top dB', type=int, default=60)
    parser.add_argument('--plot',action='store_true', help='Debug spectrogram plot')
    args = parser.parse_args()

    main(args.file[0], args.ms_frame, args.n_mels, args.top_db, args.plot)



