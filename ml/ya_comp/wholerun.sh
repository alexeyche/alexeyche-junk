#!/usr/bin/env bash
if [ ! -f parse_out ]; then
    echo "Feat counting"
    ./feat_count.sh
fi    
if [ ! -f parse_out.test ]; then
    echo "Feat for test set counting"
    ./feat_count.sh -t
fi    

if [ ! -f patterns_out ]; then
    echo "Pattern counting"
    ./ep_count.sh
fi    
if [ ! -f patterns_out.test ]; then
    echo "Pattern for test set counting"
    ./ep_count.sh -t
fi
[ ! -d episodes ] && mkdir episodes
[ ! -d episodes_test ] && mkdir episodes_test

if [ ! "`find ./episodes -type f`" ]; then
    echo "Episode distribute"
    ./episode_distributor.pl
fi    
if [ ! "`find ./episodes_test -type f`" ]; then
    echo "Episode distribute for test set"
    ./episode_distributor.pl -t
fi    
for i in `find ./episodes -type f`; do 
    filename=${i#./episodes/}
    if [ ! -f "model_$filename" ]; then
        echo "Maxent running for $filename"
        nice ./maxent/mymaxent ./episodes/$filename ./episodes_test/$filename
    else
        echo "model_$filename was found. Missing"
    fi    
done
