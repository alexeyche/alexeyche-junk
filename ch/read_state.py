#!/usr/bin/env python3

import struct

import sys

b = sys.stdin.buffer.read()

numbers = len(b) // 8

print(numbers)
print(struct.unpack("<" + "d" * numbers, b))
