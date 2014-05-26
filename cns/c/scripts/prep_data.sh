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
if [ -z $SAMPLE_SIZE ]; then
    SAMPLE_SIZE=60
fi

ADDEDNAME="_sel"

pushd $CWD &> /dev/null

../bin/snn_prep -c $CONST -i $SIMDIR/ts/synthetic_control/synthetic_control_TRAIN_${SAMPLE_SIZE}${ADDEDNAME}.bin -il $SIMDIR/ts/synthetic_control/synthetic_control_TRAIN_${SAMPLE_SIZE}${ADDEDNAME}_labels.bin -o $WORKDIR/train_spikes.bin
../bin/snn_prep -c $CONST -i $SIMDIR/ts/synthetic_control/synthetic_control_TEST_${SAMPLE_SIZE}${ADDEDNAME}.bin -il $SIMDIR/ts/synthetic_control/synthetic_control_TEST_${SAMPLE_SIZE}${ADDEDNAME}_labels.bin -o $WORKDIR/test_spikes.bin

popd &> /dev/null
