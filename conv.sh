#!/usr/bin/env bash
while read f; do 
    if ! echo $f | grep -q "utf8"; then
        iconv -f WINDOWS-1251 -t UTF-8 "$f" -o "$f.utf8.srt" 
    fi
done < <(find ./ -type f -name "*.srt")
