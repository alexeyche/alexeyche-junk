#!/usr/bin/RScript
setwd("/home/alexeyche/my/comp")

users <- read.csv("users.csv")
ev.pop.bench <- read.csv("event_popularity_benchmark.csv")
rand.bench <- read.csv("random_benchmark")
train <- read.csv("train.csv")
test <- read.csv("test.csv")
events <- read.csv("events.csv")