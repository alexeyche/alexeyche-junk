#!/usr/bin/env bash

CWD_SCR=$(readlink -f $0)
CWD=$(dirname $CWD_SCR)


function usage {
    echo "$0 -w WORK_DIR -s -l -e EPOCHS INPUT_FILE1 [INPUT_FILE2]"
}

ulimit -c unlimited

pushd $CWD &> /dev/null
SRM_SIM="../bin/snn_sim"

INPUT_FILES=
WORK_DIR=
STAT_SAVE="no"
EPOCH=
LEARN="no"
JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)

# Enumerating options
while getopts "j:w:hsle:" opt; do
    case "$opt" in
        w) WORK_DIR=${OPTARG} ;;
        s) STAT_SAVE="yes" ;;
        l) LEARN="yes" ;;
        e) EPOCH=${OPTARG} ;; 
        j) JOBS=${OPTARG} ;;
        h) usage && exit 0 ;;
        *) usage && exit 1 ;;
    esac
done    
shift $(( OPTIND - 1 ))
INPUT_FILES=${@}    

[ -z "$WORK_DIR" -o -z "$INPUT_FILES" ] && usage && exit 1
INPUT_FILES_DIR=$(for f in $INPUT_FILES; do dirname $f; done | sort -n | uniq)
INPUT_FILES_BN=$(for f in $INPUT_FILES; do basename $f; done | sort -n)
MAX_INPUT_FILES=$(echo $INPUT_FILES | wc -w)

MIN_EP=1
MAX_EP=$EPOCH

if [ -d "$WORK_DIR" ]; then
    LAST_EP=$(find $WORK_DIR -maxdepth 1 -type f -name "?*_*.bin"  -exec basename {} \; | cut -d '_' -f 1 | sort -nr | uniq | head -n 1)
    RESP="xxx"
    while true; do
        if [ "$RESP" == "y" ]; then
            MIN_EP=$((LAST_EP+1))
            MAX_EP=$((LAST_EP+EPOCH))
            break
        elif [ "$RESP" == "n" ]; then
            rm -rf $WORK_DIR/*
            cp ../snn_sim/constants.ini $WORK_DIR
            break
        else 
            read -p "$(basename $WORK_DIR) already exists and $LAST_EP epochs was done here. Continue learning? (y/n): " RESP
        fi        
    done        
else 
    mkdir -p $WORK_DIR
    cp ../snn_sim/constants.ini $WORK_DIR
fi   

function get_const {
    egrep -o "^$1.*=[ ]*[\/_.a-zA-Z0-9]+" $WORK_DIR/constants.ini | awk -F'=' '{ print $2}' | tr -d ' '
}

MEAN_P_DUR=$(get_const mean_p_dur)
LEARNING_RULE=$(get_const learning_rule)

INP_ITER=1
EPOCHS=$(seq $MIN_EP $MAX_EP)
for EP in $EPOCHS; do
    echo "epoch $EP"
    EPOCH_SFX=
    MODEL_TO_LOAD_OPT=
    if [ ! -z "$EPOCH" ]; then
        EPOCH_SFX="${EP}_"
        MODEL_TO_LOAD=$WORK_DIR/$((EP-1))_model.bin
        if [ -f "$MODEL_TO_LOAD" ]; then
            MODEL_TO_LOAD_OPT=" -ml $MODEL_TO_LOAD"
        fi        
    fi    
    LEARN=yes
    TMAX_OPT=
    if [ $EP -eq 1 ] && [ "$LEARNING_RULE" == "OptimalSTDP" ]; then
        LEARN=no
        TMAX_OPT=" -T $MEAN_P_DUR"
    fi        

    OUTPUT_SPIKES=$WORK_DIR/${EPOCH_SFX}output_spikes.bin
    OUTPUT_FILE=$WORK_DIR/${EPOCH_SFX}output.log
    MODEL_FILE=$WORK_DIR/${EPOCH_SFX}model.bin
    STAT_OPT=
    if [ "$STAT_SAVE" == "yes" ]; then
        STAT_OPT="-s $WORK_DIR/${EPOCH_SFX}stat.bin"
    fi    
    INPUT_FILE=$INPUT_FILES_DIR/$(echo $INPUT_FILES_BN | cut -d ' ' -f $INP_ITER)
    $SRM_SIM -c $WORK_DIR/constants.ini -i $INPUT_FILE -o $OUTPUT_SPIKES $STAT_OPT $MODEL_TO_LOAD_OPT -ms $MODEL_FILE -l $LEARN -j $JOBS $TMAX_OPT &> $OUTPUT_FILE
    if [ "$?" -ne 0 ]; then
        echo "Not null exit code ($?)"
        exit $?
    fi        
    INP_ITER=$((INP_ITER+1))
    if [ $INP_ITER -gt $MAX_INPUT_FILES ]; then
        INP_ITER=1
    fi        
done


popd &> /dev/null
