#!/usr/bin/env bash

names="n100_full n100_full.1 n100_full.2"

runpath="/home/alexeyche/prog/sim/runs"

for name in $names; do
    workdir="$runpath/$name"
    for ep in $(seq 1 1 33); do
        ev=$workdir/eval/$ep/eval_output.bin
        bytes=0
        if [ -f "$ev" ]; then
            bytes=$(du -b $ev  | awk '{print $1}')
        fi
        if [ "$bytes" -eq 0 ]; then
            comm="Rscript ./eval_model.R --run-name=$name --epoch=$ep"
            echo $comm
            $comm &> /dev/null
        fi               
    done
done    

