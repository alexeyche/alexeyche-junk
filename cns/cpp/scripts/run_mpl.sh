#!/usr/bin/env bash
ROOT=$(readlink -f $(dirname $0))/..
MPL_BIN=$ROOT/build/bin/mpl
TEST_R_SCRIPT=$ROOT/r_package/r_scripts/test_mpl_filter.R
INPUT_TS=~/dnn/ts/riken_14chan_3LRR.pb
MAX_EP=${MAX_EP:-200}
DIM=${DIM:-0}
RUNDIR=~/dnn/mpl/runs
EPOCHS_OF_NOISE=${EPOCHS_OF_NOISE:-900}
START_FILTER=${START_FILTER:-}
MIN_NOISE=${MIN_NOISE:-0.0}
MIN_LRATE=${MIN_LRATE:-0.0001}

mkdir -p $RUNDIR

function getvar {
    grep $1 $ROOT/mpl.json  | awk -F ':' '{print $2}' | tr -d ' ,'
}

LRATE=$(getvar "learning_rate")
THR=$(getvar "threshold")
NOISE=$(getvar "noise_sd")
FSIZE=$(getvar "filter_size")

WD=$(mktemp -d $RUNDIR/dim${DIM}_thr${THR}_fsize${FSIZE}_noise${NOISE}_lrate${LRATE}_epochs${MAX_EP}_XXXXXXXXX)
cp $ROOT/mpl.json $WD

OUT=$WD/log.out
exec > >(tee $OUT)

echo "Running in directory: $WD"
pushd $WD &>/dev/null
rm -rf *.pb *.png
NOISE_RED=( $( [ $(echo $MIN_NOISE'>'0.0 | bc -l) == "1" ] && [ $(echo $NOISE'>'0.0 | bc -l) == "1" ] && python -c "import numpy; import math; print '\n'.join([ str(math.exp(v)) for v in numpy.linspace(math.log($NOISE),math.log($MIN_NOISE), $((EPOCHS_OF_NOISE-1))) ])") $MIN_NOISE)
LRATE_RED=( $(python -c "import numpy; import math; print '\n'.join([ str(math.exp(v)) for v in numpy.linspace(math.log($LRATE),math.log($MIN_LRATE), $((EPOCHS_OF_NOISE-1))) ])") $MIN_LRATE)
[ -n $START_FILTER ] && cp $START_FILTER $WD/0_filter.pb

for i in $(seq 1 $MAX_EP); do
    cp $WD/mpl.json $WD/${i}_mpl.json
    CUR_NOISE=$NOISE
    CUR_LRATE=$LRATE
    if [ $i -lt ${#NOISE_RED[@]} ]; then
        CUR_NOISE=${NOISE_RED[$((i-1))]}
    fi        
    if [ $i -lt ${#LRATE_RED[@]} ]; then
        CUR_LRATE=${LRATE_RED[$((i-1))]}
    fi        

    sed -i -e "s|noise_sd\": [^, ]*|noise_sd\": $CUR_NOISE|g" $WD/${i}_mpl.json
    sed -i -e "s|learning_rate\": [^, ]*|learning_rate\": $CUR_LRATE|g" $WD/${i}_mpl.json
    echo -n "Epoch: $i (noise: $CUR_NOISE, lrate: $CUR_LRATE) "
    [ -f $((i-1))_filter.pb ] && cp $((i-1))_filter.pb ${i}_filter.pb
    $MPL_BIN -i $INPUT_TS -f ./${i}_filter.pb -s ${i}_spikes.pb -c $WD/${i}_mpl.json -r ./${i}_restored.pb -d $DIM | grep error | tr -d '\n' 
    echo -n " "
    #EP=$i WD=$WD R --slave -f $TEST_R_SCRIPT 2>&1 | grep Information | tr -d '\n'
    echo
done

popd 
