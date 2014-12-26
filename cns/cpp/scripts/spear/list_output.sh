#!/usr/bin/env bash

for i in $(find $(dirname $0)/output -type f); do
    echo -n $(basename $i)": "
    awk -F ':' '{ if(/eval/) { print $2$3 } }' $i |  sed -e '/^$/d' | cat -n | sort -n -k 2 -r | tail -n 1;
    echo
done | sort | sed -e '/^$/d'
