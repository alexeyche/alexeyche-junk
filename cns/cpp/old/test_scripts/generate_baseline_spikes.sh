#!/usr/bin/env bash
set -x
sudo rngd -r /dev/urandom -o /dev/random
dd if=/dev/random of=/var/tmp/random_spikes count=8192
