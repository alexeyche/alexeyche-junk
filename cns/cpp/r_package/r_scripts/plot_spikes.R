#!/usr/bin/env RScript
library(methods)

args <- commandArgs(trailingOnly = FALSE)

if(length(args) == 5) {
    cat("script usage:\n")
    cat("\tplot_spikes.R file\n")
    q()
} 
require(Rsnn)

f = args[[7]]

tmp_f = sprintf("%s.png", tempfile())
png(filename=tmp_f)
p1 = plot_rastl(RProto$new(f)$read(), T0=0,Tmax=1000)
print(p1)
dev.off()
print(tmp_f)
