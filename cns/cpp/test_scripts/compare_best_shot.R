#!/usr/bin/RScript

args <- commandArgs(trailingOnly = TRUE)
print(args)

m <- read.table("./best_shot.csv", header=FALSE, sep=",")
m <- m[,1:10]
