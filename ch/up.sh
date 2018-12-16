#!/usr/bin/env bash
set -x

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

pushd $SCRIPTPATH/local_setup

$HOME/distr/ClickHouse/build/dbms/programs/clickhouse-server

popd
