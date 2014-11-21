#!/usr/bin/env bash


CWD_SCR=$(readlink -f $0)
CWD=$(dirname $CWD_SCR)
pushd $CWD &> /dev/null

. common.sh

function usage {
    echo "$0 -s -l -e EPOCHS -v TEST_SPIKES.BIN INPUT_FILE1 [INPUT_FILE2]"
}

ulimit -c unlimited
set -x

INPUT_FILES=
WORK_DIR=
STAT_SAVE="no"
EPOCH=
LEARN="no"
AUTO="no"
LASTW="no"
EVALUATE=
EVAL_EP=1
JOBS=$(cat /proc/cpuinfo | grep -E "processor"  | wc -l)

# Enumerating options
while getopts "j:w:hsle:av:" opt; do
    case "$opt" in
        w) WORK_DIR=${OPTARG} ;;
        s) STAT_SAVE="yes" ;;
        l) LASTW="yes" ;;
        e) EPOCH=${OPTARG} ;; 
        a) AUTO="yes" ;; 
        j) JOBS=${OPTARG} ;;
        v) EVALUATE=${OPTARG} ;; 
        h) usage && exit 0 ;;
        *) usage && exit 1 ;;
    esac
done    
shift $(( OPTIND - 1 ))
INPUT_FILES=${@}    

[ -z "$INPUT_FILES" ] && usage && exit 1
if [ -z "$WORK_DIR" ]; then
    const_md5=$(md5sum ../const.json | awk '{print $1}')
    WORK_DIR_BASE=$RUNS_DIR/$const_md5
    for i in $(seq 0 1 1000); do
        WORK_DIR=${WORK_DIR_BASE}_$(printf %04d $i)
        if [ ! -d "$WORK_DIR" ]; then
            if [ $i -gt 0 ] && [ $LASTW == "yes" ]; then
                WORK_DIR=${WORK_DIR_BASE}_$(printf %04d $((i-1)))
            fi                
            break
        fi            
    done        
fi    

INPUT_FILES_DIR=$(for f in $INPUT_FILES; do dirname $f; done | sort -n | uniq)
INPUT_FILES_BN=$(for f in $INPUT_FILES; do basename $f; done | sort -n)
MAX_INPUT_FILES=$(echo $INPUT_FILES | wc -w)

MIN_EP=1
MAX_EP=$EPOCH

if [ "$AUTO" == "no" ]; then
    if [ -d "$WORK_DIR" ]; then
        LAST_EP=$(find $WORK_DIR -maxdepth 1 -type f -name "?*_*.pb"  -exec basename {} \; | cut -d '_' -f 1 | sort -nr | uniq | head -n 1)
        RESP="xxx"
        while true; do
            if [ "$RESP" == "y" ]; then
                MIN_EP=$((LAST_EP+1))
                MAX_EP=$((LAST_EP+EPOCH))
                break
            elif [ "$RESP" == "n" ]; then
                rm -rf $WORK_DIR/*
                cp ../const.json $WORK_DIR
                break
            else 
                read -p "$(basename $WORK_DIR) already exists and $LAST_EP epochs was done here. Continue learning? (y/n): " RESP
            fi        
        done        
    else 
        mkdir -p $WORK_DIR
        cp ../const.json $WORK_DIR
    fi   
fi


function evaluate {
    EP=$1
    INPUT_FILE=$INPUT_FILES_DIR/$(echo $INPUT_FILES_BN | cut -d ' ' -f 1)
    ./eval_model.sh -m $WORK_DIR/${EP}_model.bin -e $EVALUATE -t $INPUT_FILE -r 5 -s linear 
    mv -f $WORK_DIR/report.table $WORK_DIR/report_epoch_${EP};
}


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

    OUTPUT_SPIKES=$WORK_DIR/${EPOCH_SFX}output_spikes.pb
    OUTPUT_FILE=$WORK_DIR/${EPOCH_SFX}output.log
    MODEL_FILE=$WORK_DIR/${EPOCH_SFX}model.pb
    STAT_OPT=
    if [ "$STAT_SAVE" == "yes" ]; then
        STAT_OPT="--stat $WORK_DIR/${EPOCH_SFX}stat.pb"
    fi    
    INPUT_FILE=$INPUT_FILES_DIR/$(echo $INPUT_FILES_BN | cut -d ' ' -f $INP_ITER)
    $SNN_SIM -c $WORK_DIR/const.json -i $INPUT_FILE -o $OUTPUT_SPIKES $STAT_OPT -j $JOBS &> $OUTPUT_FILE
    if [ "$?" -ne 0 ]; then
        echo "Not null exit code ($?)"
        exit $?
    fi        
    INP_ITER=$((INP_ITER+1))
    if [ $INP_ITER -gt $MAX_INPUT_FILES ]; then
        INP_ITER=1
    fi        
    LAST_EP=$EP
    if [ -n "$EVALUATE" ] && ( [ $(echo "$EP % $EVAL_EP" | bc) -eq 0 ] || [ $EP -eq 1 ] ); then
        evaluate $EP
    fi
done

if [ -n "$EVALUATE" ] && [ ! $(echo "$EP % $EVAL_EP" | bc) -eq 0 ]; then
    evaluate $EP
fi    

popd &> /dev/null
