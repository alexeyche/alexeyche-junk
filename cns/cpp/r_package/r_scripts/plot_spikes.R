#!/usr/bin/env Rscript
library(methods)

args <- commandArgs(trailingOnly = FALSE)
args_i = grep("--args", args)

if(args_i == 0) {
    cat("script usage:\n")
    cat("\tplot_spikes.R file\n")
    q()
} 
require(Rsnn)
f = args[[args_i+1]]

plot_rastl(RProto$new(f)$read(), T0=0,Tmax=1000)

message("Press Return To Continue")
invisible(readLines("stdin", n=1))
#dev.off()
#print(tmp_f)
