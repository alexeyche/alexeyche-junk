#!/usr/bin/env bash
set -ex
cur_dir=$(readlink -f $(dirname $0))
pushd $cur_dir &>/dev/null
mkdir -p build/py
protoc --python_out=build/py ./sources/dnn/protos/*.proto
for d in $(find build/py -type d); do
    touch $d/__init__.py
done
popd $cur_dir &>/dev/null
