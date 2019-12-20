#!/usr/bin/env bash


CH="/Users/aleksei/distr/ClickHouse/build/dbms/programs/clickhouse client"

pushd /Users/aleksei/distr/alexeyche-junk/ch

cat ./dump_state.sql | $CH -h 0.0.0.0 | ./read_state.py

popd