#!/usr/bin/env bash
CWD_SCR=$(readlink -f $0)
CWD=$(dirname $CWD_SCR)

ulimit -c unlimited

pushd $CWD &> /dev/null
SNN_SIM="../bin/snn_sim"
SNN_POSTPROC="../bin/snn_postproc"

function usage {
    echo "$0 -m MODEL -e TEST_SPIKES -t TRAIN_SPIKES -r REPEATS -s SCHEME(METHOD)"
    echo "Methods:"
    echo -e "\tsvm"
    echo -e "\tspike_sort"
}
MODEL=
EVALUATE=
TRAIN_SPIKES=
REPEATS=1
METHOD="svm"
# Enumerating options
while getopts "m:t:e:r:s:" opt; do
    case "$opt" in
        m) MODEL=$(readlink -f ${OPTARG}) ;; 
        e) EVALUATE=${OPTARG} ;; 
        t) TRAIN_SPIKES=${OPTARG} ;; 
        r) REPEATS=${OPTARG} ;; 
        s) METHOD=${OPTARG} ;;
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

function train_fun {
    ~/distr/liblinear-1.94/train -s 0 -c 0.1 $1 ${1%.*}.model
    ~/distr/liblinear-1.94/predict $2 ${1%.*}.model $3
}
function get_value_from_log {
    grep -E "$2" $1 | cut -d '=' -f 2 | grep -Eo "^[ ]*[^ ]+" | tr -d ' %'
}

ACCVAL=0
M=$(get_const M)
REPEAT_NUMBERS=$(seq 1 1 $REPEATS)
if [ -d "$WORK_DIR/eval" ]; then
    rm -rf "$WORK_DIR/eval"
fi    
mkdir "$WORK_DIR/eval"
LO_K=5
HI_K=100
IT_K=5
for i in $REPEAT_NUMBERS; do 
    MODEL_TO_LOAD_OPT=" -ml $MODEL"
    OUTPUT_SPIKES_TRAIN=$WORK_DIR/eval_output_spikes_train.bin
    OUTPUT_FILE="$WORK_DIR/eval_output_${i}.log"
    [ -f $OUTPUT_FILE ] && rm -f $OUTPUT_FILE
    $SNN_SIM -c $WORK_DIR/constants.ini -i $TRAIN_SPIKES -o $OUTPUT_SPIKES_TRAIN $MODEL_TO_LOAD_OPT -l no -j $JOBS &> $OUTPUT_FILE  
    OUTPUT_SPIKES_TEST=$WORK_DIR/eval_output_spikes_test.bin
    $SNN_SIM -c $WORK_DIR/constants.ini -i $EVALUATE -o $OUTPUT_SPIKES_TEST $MODEL_TO_LOAD_OPT -l no -j $JOBS  &>> $OUTPUT_FILE  
    if [ "$METHOD" == "svm" ]; then
        $SNN_POSTPROC -i $OUTPUT_SPIKES_TRAIN -t $OUTPUT_SPIKES_TEST -o $WORK_DIR/eval/postproc_$i -k $LO_K:$IT_K:$HI_K --svm-out &>> $OUTPUT_FILE
        for k in $(seq $LO_K $IT_K $HI_K); do
            k_f=$(printf "%3.1f" $k)
            echo "Kernel = $k_f" >> $OUTPUT_FILE
            train_f=$WORK_DIR/eval/postproc_${i}.train.k${k_f}.dat
            test_f=$WORK_DIR/eval/postproc_${i}.test.k${k_f}.dat
            predict_f=${test_f%.*}.predict
            train_fun $train_f $test_f $predict_f &>> $OUTPUT_FILE
            paste -d ' ' <(cut -d ' ' -f 1 $test_f | grep -E '[0-9]+') <(cut -d ' ' -f 1 $predict_f | grep -E '[0-9]+') > $predict_f.table
            $SNN_POSTPROC --calc-predict-stat $predict_f.table &>> $OUTPUT_FILE
        done
        echo -e "Kernel\tNMI\tAccuracy" > $WORK_DIR/report.table.${i}
        paste <( get_value_from_log $OUTPUT_FILE "Kernel" )   \
              <( get_value_from_log $OUTPUT_FILE "NMI" )      \
              <( get_value_from_log $OUTPUT_FILE "Accuracy" ) \
              >> $WORK_DIR/report.table.${i}
    elif [ "$METHOD" == "spike_sort" ]; then
        $SNN_POSTPROC -i $OUTPUT_SPIKES_TRAIN -t $OUTPUT_SPIKES_TEST -o $WORK_DIR/eval/postproc_$i -k 10 --ignore-first-neurons $M &>> $OUTPUT_FILE
    fi        
done
if [ "$METHOD" == "svm" ]; then

    Rscript --no-init-file ./mean_report_table.R $WORK_DIR/report.table.* > $WORK_DIR/report.table
elif [ "$METHOD" == "spike_sort" ]; then
    echo "pass"
fi

popd &> /dev/null
