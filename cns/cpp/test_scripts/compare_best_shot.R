#!/usr/bin/RScript

cmd_args = commandArgs();
for (arg in cmd_args) cat("  ", arg, "\n", sep="");
m <- read.table("./best_shot.csv", header=FALSE, sep=",")
m <- m[,1:10]
