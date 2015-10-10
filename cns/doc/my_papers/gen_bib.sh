#!/usr/bin/env bash
set -x
[ -z $1 ] && exit 1
[ -f $1 ] || exit 1
cp $1 ${1}.bak
bn=${1%%.tex} 
sed -i -e 's|^\\includegraphics|%\\includegraphics|g' $1
latex  ./$bn && bibtex $bn && latex $bn && latex $bn
mv ${1}.bak review.tex
