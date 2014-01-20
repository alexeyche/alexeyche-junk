#!/usr/bin/env bash


while read f; do 
    newname=$(echo $f | grep -oE '2x[0-9]+')
    iconv "$f" -f WINDOWS-1251 -t UTF8 -o the.doctor.utf8.$newname.srt; 
done < <( find ./ -type f -name "*.srt" )
