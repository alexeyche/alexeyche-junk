#!/usr/bin/env bash
SIMDIR=/home/alexeyche/prog/sim

CWD_SCR=$(readlink -f $0)
CWD=$(dirname $CWD_SCR)

CONST="../snn_sim/constants.ini"

if [ -z $WORKDIR ]; then
    WORKDIR=$SIMDIR/spikes/ucr
else
    CONST=$WORKDIR/constants.ini
fi

pushd $CWD &> /dev/null

../bin/snn_prep -c $CONST -i $SIMDIR/ts/synthetic_control/synthetic_control_TRAIN_1000.bin -il $SIMDIR/ts/synthetic_control/synthetic_control_TRAIN_1000_labels.bin -o $WORKDIR/train_spikes.bin
../bin/snn_prep -c $CONST -i $SIMDIR/ts/synthetic_control/synthetic_control_TEST_1000.bin -il $SIMDIR/ts/synthetic_control/synthetic_control_TEST_1000_labels.bin -o $WORKDIR/test_spikes.bin

popd &> /dev/null
