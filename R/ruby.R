require(tm)
require(SnowballC)
setwd("~/prog/alexeyche-junk/R")

(ruby <- Corpus(DirSource("Ruby", encoding = "UTF-8"), readerControl = list(language = "english")))
r = ruby[[1]]
r = removePunctuation(r)
r = tolower(r)
r = stripWhitespace(r)
#r = removeWords(r, stopwords("english"))
#r = stemDocument(r)
ruby[[1]] = r
m <- as.matrix(TermDocumentMatrix(ruby))
d <- data.frame(word = names(m[,1]),freq=m[,1])
png("ruby.png",width=1280, height=1280)
wordcloud(d$word, d$freq, scale=c(15,0.5), min.freq=2,max.words=1000, random.order=TRUE, vfont=c("sans serif","plain"))
dev.off()