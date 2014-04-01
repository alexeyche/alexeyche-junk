#!/usr/bin/env bash

SRM_SIM="../bin/srm_sim"
#WORK_DIR=~/prog/sim/cprog/runs/rfd
WORK_DIR=~/my/sim/cprog/runs/rfd
RFD_DIR=/home/alexeyche/my/sim/rfd_files

mkdir -p $WORK_DIR

rm -rf $WORK_DIR/*
cp ../srm_sim/constants.ini $WORK_DIR
CONST_INI="$WORK_DIR/constants.ini"



DURATION_OF_INPUT=30000

function get_const {
    egrep -o "^$1.*=[ ]*[\/_.a-zA-Z0-9]+" $CONST_INI | awk -F'=' '{ print $2}' | tr -d ' '
}


INPUT_FILE=`get_const input_spikes_filename`
MEAN_P_DUR=`get_const mean_p_dur`
EPOCHS=`get_const epochs`


DUR=0

INP_NUM=1
LAST_MODEL=
COLLECT_STAT=yes
COLLECT_STAT=
for EP in {1..100}; do
    OUTPUT_SPIKES="$WORK_DIR/${EP}_output_spikes.bin"
    STAT_FILE="$WORK_DIR/${EP}_stat.bin"
    MODEL_FILE="$WORK_DIR/${EP}_model.bin"
    OUTPUT_FILE="$WORK_DIR/${EP}_output.sim"
    LEARN=yes
    if [ $DUR -lt $MEAN_P_DUR ]; then
        LEARN=no
    fi        
    MODEL_TO_LOAD_OPT=
    if [ ! -z $LAST_MODEL ]; then 
        MODEL_TO_LOAD_OPT="-ml $LAST_MODEL"
    fi
    STAT_OPT=
    if [ "$COLLECT_STAT" == "yes" ]; then
        STAT_OPT=" -s $STAT_FILE"
    fi        
    INPUT_FILE="$RFD_DIR/ep_${INP_NUM}_30.0sec.bin"
    $SRM_SIM -c $CONST_INI -o $OUTPUT_SPIKES -l $LEARN -ms $MODEL_FILE  -i $INPUT_FILE $STAT_OPT $MODEL_TO_LOAD_OPT &> $OUTPUT_FILE

    echo "epoch $EP done"
    DUR=$((DUR+DURATION_OF_INPUT))
    INP_NUM=$((INP_NUM+1))
    if [ $INP_NUM -eq 101 ]; then
        INP_NUM=1
    fi        
    LAST_MODEL="$MODEL_FILE"
done
