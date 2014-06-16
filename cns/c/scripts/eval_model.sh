#!/usr/bin/env bash
CWD_SCR=$(readlink -f $0)
CWD=$(dirname $CWD_SCR)

ulimit -c unlimited

pushd $CWD &> /dev/null
SNN_SIM="../bin/snn_sim"
SNN_POSTPROC="../bin/snn_postproc"

function usage {
    echo "$0 -m MODEL -e TEST_SPIKES -t TRAIN_SPIKES"
}
MODEL=
EVALUATE=
TRAIN_SPIKES=
REPEATS=1
# Enumerating options
while getopts "m:t:e:r:" opt; do
    case "$opt" in
        m) MODEL=$(readlink -f ${OPTARG}) ;; 
        e) EVALUATE=${OPTARG} ;; 
        t) TRAIN_SPIKES=${OPTARG} ;; 
        r) REPEATS=${OPTARG} ;; 
        h) usage && exit 0 ;;
        *) usage && exit 1 ;;
    esac
done    
([ -z $MODEL ] || [ -z $EVALUATE ] || [ -z $TRAIN_SPIKES ]) && usage && exit 1

WORK_DIR=$(dirname $MODEL)
JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)
function get_const {
    egrep -o "^$1.*=[ ]*[\/_.a-zA-Z0-9]+" $WORK_DIR/constants.ini | awk -F'=' '{ print $2}' | tr -d ' '
}
ACCVAL=0
M=$(get_const M)
REPEAT_NUMBERS=$(seq 1 1 $REPEATS)
for i in $REPEAT_NUMBERS; do 
    MODEL_TO_LOAD_OPT=" -ml $MODEL"
    OUTPUT_SPIKES_TRAIN=$WORK_DIR/eval_output_spikes_train.bin
    OUTPUT_FILE="eval_output.log"
    $SNN_SIM -c $WORK_DIR/constants.ini -i $TRAIN_SPIKES -o $OUTPUT_SPIKES_TRAIN $MODEL_TO_LOAD_OPT -l no -j $JOBS &> $OUTPUT_FILE  
    OUTPUT_SPIKES_TEST=$WORK_DIR/eval_output_spikes_test.bin
    $SNN_SIM -c $WORK_DIR/constants.ini -i $EVALUATE -o $OUTPUT_SPIKES_TEST $MODEL_TO_LOAD_OPT -l no -j $JOBS &> $OUTPUT_FILE  
    VAL=$( $SNN_POSTPROC -i $OUTPUT_SPIKES_TRAIN -t $OUTPUT_SPIKES_TEST -o $WORK_DIR/postproc_stat.bin -k 5:5:100 --ignore-first-neurons $M )
    ACCVAL=$(echo $ACCVAL + $VAL | bc)
done
echo "scale=10; $ACCVAL/$REPEATS" | bc
popd &> /dev/null
