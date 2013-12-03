#!/usr/bin/RScript

require(R.matlab)

topic_toolbox <-"/home/alexeyche/prog/topictoolbox/"
#topic_toolbox <- "/home/alexeyche/my/dbn/topic_mod/topictoolbox/"
bag <- readMat(paste(topic_toolbox , 'bagofwords_nips.mat', sep="") )
words <- readMat(paste(topic_toolbox , 'words_nips.mat', sep=""))
titles <- readMat(paste(topic_toolbox , 'titles_nips.mat', sep=""))
authors <- readMat(paste(topic_toolbox ,'authors_nips.mat', sep=""))

