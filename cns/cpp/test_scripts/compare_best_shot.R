#!/usr/bin/RScriptq

args <- commandArgs(trailingOnly = TRUE)

if(length(args) == 0) {
    quit()
}
fname = "/home/alexeyche/my/git/alexeyche-junk/cns/cpp/test_scripts/best_shot.csv"
m <- read.table(fname, header=FALSE, sep=",")
m2 <- read.table(args, header=FALSE, sep=",")
m <- m[,1:10]
m2 <- m2[,1:10]
cat(sum(abs(m2-m)),"\n")
quit()
