set -x
if [ ! -f patterns_out ]; then
    ./parse_patterns.pl dataset/train > patterns_out
fi    
if [ ! -f episode_out ]; then 
    rm -rf ./episode_out*
    ./episode_analyser.pl patterns_out > episode_out
    ./freq_analyser.pl episode_out > episode_out.freq
    sort -t '	' -k 2 -n -r ./episode_out.freq > episode_out.freq.sort
fi     
