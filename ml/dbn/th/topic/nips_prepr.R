#!/usr/bin/RScript

require(R.matlab)

bag <- readMat('/home/alexeyche/my/dbn/topic_mod/topictoolbox/bagofwords_nips.mat') 
words <- readMat('/home/alexeyche/my/dbn/topic_mod/topictoolbox/words_nips.mat')
titles <- readMat('/home/alexeyche/my/dbn/topic_mod/topictoolbox/titles_nips.mat')
authors <- readMat('/home/alexeyche/my/dbn/topic_mod/topictoolbox/authors_nips.mat')

# $ WS: num [1, 1:2301375] 1 1 1 1 1 1 1 1 1 1 ...
# $ DS: num [1, 1:2301375] 1 1 1 1 1 1 1 1 1 1 ...
# - attr(*, "header")=List of 3
dict_size <- max(bag$WS)
doc_size <- max(bag$DS)

cur_doc <- 0
docs <- array(0, dim=c(doc_size, dict_size))

for(i in 1:length(bag$DS)) {
    if(cur_doc != bag$DS[1,i]) {
        cur_doc <- bag$DS[1,i]
    }
    cur_word <- bag$WS[1,i]
    docs[cur_doc, cur_word] <- docs[cur_doc, cur_word] + 1
}

write.table(docs, "nips_feats.csv", sep=",", row.names=FALSE, col.names=FALSE)
