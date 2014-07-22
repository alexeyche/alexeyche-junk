#!/usr/bin/Rscript

args <- commandArgs(trailingOnly = FALSE)
if(length(args) == 5) {
    cat("Available options: \n")
    cat("\t\t--dir-with-prep-spikes=%subj%\n")
}
spikes_dir = substring( args[grep("--dir-with-prep-spikes=", args)], 24)

m = loadMatrix(sprintf("%s/train_spikes", spikes_dir), 1)
str(m)
