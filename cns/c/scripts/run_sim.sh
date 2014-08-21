#!/usr/bin/env bash
set -x

CWD_SCR=$(readlink -f $0)
CWD=$(dirname $CWD_SCR)


function usage {
    echo "$0 -w WORK_DIR -s -l -e EPOCHS -v TEST_SPIKES.BIN INPUT_FILE1 [INPUT_FILE2]"
}
function get_const_for_name {
   grep -Eo "^$1.*=[ ]*[ \/_.a-zA-Z0-9]+" ../snn_sim/constants.ini | awk -F'=' '{ print $2}' | sed -e 's|^[ ]*||g' -e 's|[ ]*$||g'  | tr ' ' _
}

ulimit -c unlimited

pushd $CWD &> /dev/null
SNN_SIM="../bin/snn_sim"
SNN_POSTPROC="../bin/snn_postproc"
RUNS_DIR=~/prog/sim/runs

INPUT_FILES=
WORK_DIR=
STAT_SAVE="no"
EPOCH=
LEARN="no"
AUTO="no"
LASTW="no"
EVALUATE=
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
    WORK_DIR_BASE=$RUNS_DIR/$(get_const_for_name neuron_type)_$(get_const_for_name learning_rule)_$(get_const_for_name prob_fun)
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
fi

function get_const {
    egrep -o "^$1.*=[ ]*[\/_.a-zA-Z0-9]+" $WORK_DIR/constants.ini | awk -F'=' '{ print $2}' | tr -d ' '
}


MEAN_P_DUR=$(get_const mean_p_dur)
TAU_AVERAGE=$(get_const tau_average)
LEARNING_RULE=$(get_const learning_rule)
REINFORCEMENT=$(get_const reinforcement)

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

    OUTPUT_SPIKES=$WORK_DIR/${EPOCH_SFX}output_spikes.bin
    OUTPUT_FILE=$WORK_DIR/${EPOCH_SFX}output.log
    MODEL_FILE=$WORK_DIR/${EPOCH_SFX}model.bin
    STAT_OPT=
    if [ "$STAT_SAVE" == "yes" ]; then
        STAT_OPT="-s $WORK_DIR/${EPOCH_SFX}stat.bin --stat-level 1"
    elif [[ "$REINFORCEMENT" =~ "true" ]]; then
        STAT_OPT="--stat-level 1 -s $WORK_DIR/${EPOCH_SFX}stat.bin"
    fi    
    INPUT_FILE=$INPUT_FILES_DIR/$(echo $INPUT_FILES_BN | cut -d ' ' -f $INP_ITER)
    $SNN_SIM -c $WORK_DIR/constants.ini -i $INPUT_FILE -o $OUTPUT_SPIKES $STAT_OPT $MODEL_TO_LOAD_OPT -ms $MODEL_FILE -j $JOBS &> $OUTPUT_FILE
    if [ "$?" -ne 0 ]; then
        echo "Not null exit code ($?)"
        exit $?
    fi        
    INP_ITER=$((INP_ITER+1))
    if [ $INP_ITER -gt $MAX_INPUT_FILES ]; then
        INP_ITER=1
    fi        
    LAST_EP=$EP
done

if [ -n "$EVALUATE" ]; then
    INPUT_FILE=$INPUT_FILES_DIR/$(echo $INPUT_FILES_BN | cut -d ' ' -f 1)
    ./eval_model.sh -m $WORK_DIR/${LAST_EP}_model.bin -e $EVALUATE -t $INPUT_FILE 
fi    

popd &> /dev/null
