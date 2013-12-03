#!/usr/bin/PScript

TRAIN_FILE="training.txt"
FEAT_FILE="training_feat.csv"
WORD_COUNT=2287
word_corpus = {}
word_id_to_count = {}
all_feats = []
f = open(TRAIN_FILE, 'r')
ff = open(FEAT_FILE,'w')
l_num = 0
word_id = 1
for line in f:
    l_num+=1
    spl = line.split('\t')
    words = spl[1].split(' ')
    feats = [int(spl[0])] + [0]*WORD_COUNT
    for w in words:
#'\xe2\x80\x98': 2, '\xe2\x80\x9d': 5, '\xe2\x80\x9c': 3,
        if (w == '\xe2\x80\x99') or (w == '\xe2\x80\x98'):
            continue # it is ` character
        if (w == '\xe2\x80\x9d') or (w == '\xe2\x80\x9c'):
            continue # it is '' character
        if (w == '\xef\xbf\xbd') or (w == '\xe2\x80\x93'):
            continue
        w0 = w.strip('!,.:)(\n\t><\'"&')
        if not w0:
            continue
        w1 = w0.lower()
        if w1 not in word_corpus:
            word_corpus[w1] = word_id
            word_id+=1
            word_id_to_count[word_corpus[w1]] = 1
        word_id_to_count[word_corpus[w1]] +=1
        feats[word_corpus[w1]] += 1
    all_feats.append(feats)    

rare_words = []
for id in word_id_to_count:
    if word_id_to_count[id]<3:
        rare_words.append(id)

good_words = [ i for i in xrange(1,2288) if i not in rare_words ]

for feats in all_feats:
    filtered = [feats[i] for i in good_words]
    string = ','.join(map(str,filtered))
    string += '\n'
    ff.write(string)

f.close()
ff.close()
#print
#print 'total words = %s' % (word_id)
