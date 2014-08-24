#!/usr/bin/env bash
cp ./review.tex review.tex.bak
sed -i -e 's|^\\includegraphics|%\\includegraphics|g' ./review.tex
latex  ./review && bibtex ./review && latex  ./review && latex  ./review
mv review.tex.bak review.tex
