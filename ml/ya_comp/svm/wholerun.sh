#!/usr/bin/env bash
dataset_dir=$1
episode_file=$2
if [ ! -d "$dataset_dir" ]; then  echo "need dataset dir"; exit 1; fi
if [ ! -f parse_out.svm ]; then 
    echo "train parse for svm started"
  ( ( ./parse_svm.pl $dataset_dir/train > parse_out.svm ) && 
    ( echo "train parse for svm finished" ) && 
    (./svm-scale parse_out.svm >  parse_out.svm.scale) && 
    (echo "scaling train finished") )&
else
    echo "parse_out.svm was found. missing"
fi
if [ ! -f parse_out.svm.test ]; then 
    echo "test parse for svm started"
  ( ( ./parse_svm.pl $dataset_dir/test -t > parse_out.svm.test ) && 
    ( echo "test parse for svm finished" ) && 
    (./svm-scale parse_out.svm.test >  parse_out.svm.test.scale) && 
    (echo "scaling test finished") ) &

fi    

wait

[ ! -d episodes ] && mkdir episodes

if [ ! "`find ./episodes -type f`" ]; then
    echo "lets distribute episodes"
    ./episode_distributor_svm.pl 
    echo "lets distribute episodes for test"
    ./episode_distributor_svm.pl -t
fi
for i in `find ./episodes -type f`; do 
    filename=${i#./episodes/}
    ( ([ ! -f ./episodes/${filename}_sid ]) && (awk -F ' ' '{ $1 = 1; print }' $i > ./episodes/${filename}_sid) ) & 
    if [ ! -f "models/model_$filename" ]; then
        [ ! -d models ] && mkdir models
        echo "SVM running for $filename"
        nice ./svm-train -b 1 -h 0 $i models/model_$filename
    else
        echo "model_$filename was found. Missing"
    fi
done




