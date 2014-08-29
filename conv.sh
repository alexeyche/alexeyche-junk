#!/usr/bin/env bash
while read f; do 
    iconv -f WINDOWS-1251 -t UTF-8 "$f" -o "$f.utf8.srt" 
done < <(find ./ -type f -name "*.srt")
