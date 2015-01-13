#!/usr/bin/env bash
SPEARMINT="/home/alexeyche/distr/Spearmint/spearmint"
EXP_SOURCE_DIR=$1

if [ -z $EXP_SOURCE_DIR ] || [ ! -d $EXP_SOURCE_DIR ]; then
    echo "Need existing experiment source dir as first parameter"
    exit 1
fi    

rm -rf $EXP_SOURCE_DIR/output 
mongo spearmint --eval "db.dropDatabase();" 
python $SPEARMINT/main.py $EXP_SOURCE_DIR
