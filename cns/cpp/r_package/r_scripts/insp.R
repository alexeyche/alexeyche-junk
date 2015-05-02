#!/usr/bin/env Rscript
library(utils)
library(base)
library(graphics)
library(stats)
library(datasets)
library(grDevices)
library(methods)

require(Rdnn)
require(rjson)

PIC_TOOL = "eog -f"

EP = convNum(Sys.getenv('EP'), -1)
WD = convStr(Sys.getenv('WD'), getwd())
T0 = convNum(Sys.getenv('T0'), 0)
T1 = convNum(Sys.getenv('T1'), 1000)

args <- commandArgs(trailingOnly = FALSE)
if(length(grep("RStudio", args))>0) {
    WD = sprintf("~/dnn/runs/%s", system("ls -t ~/dnn/runs | head -n 1", intern=TRUE))
    EP=13
    T1=10000
}

pfx_f = function(s) s
if(EP>=0) {
    pfx_f = function(s) sprintf("%d_%s", EP, s)
}


CONST_FNAME = convStr(Sys.getenv('CONST'), "const.json")
MODEL_FNAME = convStr(Sys.getenv('MODEL'), pfx_f("model.pb"))
SPIKES_FNAME = convStr(Sys.getenv('SPIKES'), pfx_f("spikes.pb"))
STAT_FNAME = convStr(Sys.getenv('STAT'), pfx_f("stat.pb"))
SP_PIX0 = convNum(Sys.getenv('SP_PIX0'), 1024)
SP_PIX1 = convNum(Sys.getenv('SP_PIX1'), 768)
STAT_ID = convNum(Sys.getenv('STAT_ID'), 0) + 1 # C-like indices
STAT_SYN_ID = convNum(Sys.getenv('STAT_SYN_ID'), NULL)
COPY_PICS = convStr(Sys.getenv('COPY_PICS'), "no") %in% c("yes", "1", "True", "true")
OPEN_PIC = convStr(Sys.getenv('OPEN_PIC'), "yes") %in% c("yes", "1", "True", "true")



if(length(grep("RStudio", args))>0) {
    STAT_SYN_ID=78
}

setwd(WD)


tmp_d = Rdnn.tempdir()

input = NULL
model = NULL
net = NULL

if(file.exists(CONST_FNAME)) {
    const = fromJSON(file=CONST_FNAME)
    inputs = sapply(const$sim_configuration$files, function(x) x$filename)
    for(i in inputs) {
        i = gsub("^@", "", i)
        i = gsub("-", "_", i)
        ifile = sprintf("%s.pb", i)
        if(file.exists(ifile)) {
            if(!is.null(input)) {
                stop("Can't deal with multiple inputs")
            }
            input = RProto$new(ifile)$read()
        }
    }
}


pic_files = NULL

if(file.exists(SPIKES_FNAME)) {
    net = RProto$new(SPIKES_FNAME)$read()
    spikes_pic = sprintf("%s/1_%s", tmp_d, pfx_f("spikes.png"))
    png(spikes_pic, width=SP_PIX0, height=SP_PIX1)
    pspikes = prast(net$values, T0=T0,Tmax=T1)
    
    
    print(pspikes)
    
    dev.off()
        
    cat("Spikes pic filename: ", spikes_pic, "\n")
    pic_files = c(pic_files, spikes_pic)
} else {
    warning(sprintf("Not found %s", SPIKES_FNAME))
}

if (file.exists(MODEL_FNAME)) {
    model = RProto$new(MODEL_FNAME)$read()
    w = matrix(0, nrow=length(model), ncol=length(model))
    for(n in model) {
        w[n$id+1, n$synapses$ids_pre+1] = n$synapses$weights
    }
    
    weights_pic = sprintf("%s/2_%s", tmp_d, pfx_f("weights.png"))
    png(weights_pic, width=1024, height=768)
    print(gr_pl(w))
    dev.off()
    
    cat("Weights pic filename: ", weights_pic, "\n")
    pic_files = c(pic_files, weights_pic)
    
#     lsize = sapply(const$sim_configuration$layers, function(x) x$size)
#     maps = getWeightMaps(5,5, w, lsize)
#     weight_map_pic = sprintf("%s/4_%s", tmp_d, pfx_f("weight_map.png"))
#     png(weight_map_pic, width=1024, height=768)
#     print(gr_pl(maps[[length(maps)]]))
#     dev.off()
#     pic_files = c(pic_files, weight_map_pic)
} else {
    warning(sprintf("Not found %s", MODEL_FNAME))
}

if (file.exists(STAT_FNAME)) {
    stat = RProto$new(STAT_FNAME)$rawRead()        
    stat_pic = sprintf("%s/3_%s", tmp_d, pfx_f("stat.png"))
    png(stat_pic, width=1024, height=768*6)
    
    plot_stat(stat[[STAT_ID]], STAT_SYN_ID, T0, T1)
    
    dev.off()
    cat("Stat pic filename: ", stat_pic, "\n")
    pic_files = c(pic_files, stat_pic)
} else {
    warning(sprintf("Not found %s", STAT_FNAME))
}

if ( (!is.null(input))&&(!is.null(model))&&(!is.null(net)) ) {
    for(sp in net$values) {
        lab = 
        for(sp_t in sp) {
                
        }        
    }
    input$ts_info    
}


if(COPY_PICS) {
    new_pic_files = NULL
    for(p in pic_files) {
        dst = sprintf("%s/%s", getwd(), basename(p))
        file.copy(p, dst)
        new_pic_files = c(new_pic_files, dst)
    }
    pic_files = new_pic_files
}

if((length(pic_files)>0)&&(OPEN_PIC)) {
    open_pic(pic_files[1])
}