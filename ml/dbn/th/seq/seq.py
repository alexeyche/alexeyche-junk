#!/usr/bin/env python

symb = ["Q", "Qbad1", "Qbad2", "Qbad3", "C0", "C1", "Cb0", "Cb1", "S"]

train_file = "/home/alexeyche/my/prog/ya_sw_det/patterns_switch"

rec = len(symb) * ["0"]

for line in open(train_file,"r"):
    line = line.rstrip()
    l = line.split(",")
    for sym in l:
        r = list(rec)
        index = symb.index(sym)
        if index>=0:
            r[index] = "1"
            print ",".join(r)
        else:
            print "%s not found" % sym
            quit(1)
    r = list(rec)
    print ",".join(r)

