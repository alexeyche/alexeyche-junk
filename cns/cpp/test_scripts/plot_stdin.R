#!/usr/bin/RScript

f <- file("stdin")
s <- readLines(f)
d <- as.double(strsplit(s, ",")[[1]])
dt <- 0.1
x11()
plot( (1:length(d))*dt, d, type="l")
Sys.sleep(10)
