#!/usr/bin/env bash

cat ~/distr/junk/ch/q.sql | \
    /Users/aleksei/distr/ClickHouse/build/dbms/programs/clickhouse client -h 0.0.0.0 | \
    ~/distr/junk/ch/read.py