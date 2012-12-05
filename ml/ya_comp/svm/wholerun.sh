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
    (echo "scaling test finished") &&
    (awk -F ' ' '{ $1 = 1; print }' parse_out.svm.test.scale > parse_out.svm.test.scale.class ) ) & #& 
fi    


wait

[ ! -d episodes ] && mkdir episodes
[ ! -d episodes_strat ] && mkdir episodes_strat
[ ! -d episodes_quest ] && mkdir episodes_quest

if [ ! "`find ./episodes -type f`" ]; then
    echo "lets distribute episodes"
    ./episode_distributor_svm.pl 
    echo "lets distribute episodes for test"
    ./episode_distributor_svm.pl -t
    echo "lets distribute episodes for questions"
    ./episode_distributor_svm.pl -q
fi


STRAT_LIM=10000

for i in `find ./episodes -type f`; do 
    filename=${i#./episodes/}    
    [ -f ./episodes_strat/$filename ] && continue
    lines=`cat $i | wc -l`
    echo -n "$i : $lines, so"
    ( ([ ! -f ./episodes_quest/${filename} ]) && ( sed -ne '/^1/p' $i > ./episodes_quest/${filename} ) ) & 
    if [ $lines -ge $STRAT_LIM ]; then
        echo " strat"
        python ../tools/subset.py $i $STRAT_LIM ./episodes_strat/${filename}
    else 
        echo " copy"
        cp $i ./episodes_strat
    fi 
done


for i in `find ./episodes_strat -type f`; do 
    filename=${i#./episodes_strat/}
    if [ ! -f "models/model_$filename" ]; then
        [ ! -d models ] && mkdir models
        echo "SVM running for $filename"
        nice ./svm-train -b 1 -h 0 $i models/model_$filename
    else
        echo "model_$filename was found. Missing"
    fi
done    

# prediction

[ ! -d prediction ] && mkdir prediction

for i in `find ./models -type f`; do 
    modelname=${i#./models/model_}
    if [ -f ./episodes_quest/$modelname ]; then
        ./svm-predict -b 1 ./episodes_quest/$modelname $i ./prediction/$modelname
        tmpfile=`mktemp`
        cut -d ' ' -f 2 ./prediction/$modelname > $tmpfile
        paste -d ' ' ./episodes_quest/$modelname $tmpfile > ./prediction/${modelname}_answer
        rm -rf $tmpfile
        echo "$modelname prediction done"
    else 
        echo "answer for $modelname was found. missing"
    fi        
done    




