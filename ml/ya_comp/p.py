#!/usr/bin/env python


class SessionMeta:
    def __init__(self,id,day,user,sw):
        self.id = id
        self.day = day
        self.user = user
        self.sw = sw
    def __repr__(self):
        return "%s %s %s %s\n" % (self.id, self.day, self.user, self.sw)


class Query:
    def __init__(self,time,serp_id,query_id,urls):
        self.time = time
        self.serp_id = serp_id
        self.query_id = query_id
        self.urls = urls
    def __repr__(self):
        return "Query: %s %s %s [ %s ]\n" % (self.time, self.serp_id, self.query_id, ', '.join(self.urls),)


class Click:
    def __init__(self,time,serp_id,url_id):
        self.time = time
        self.serp_id = serp_id
        self.url_id = url_id
    def __repr__(self):
        return "Click: %s %s %s\n" % (self.time, self.serp_id, self.url_id,)

class Switch:
    def __init__(self,time):
        self.time = time
    def __repr__(self):
        return "Switch: %s\n" % (self.time,)

f = open('train.less','r') 
lines = f.readlines()
blocks = {}
block_add = True
for line in lines:
    line_w = line.rstrip('\n')
    line_s = line_w.split('\t')
    sess_id = line_s[0]
    type_rec = line_s[2]
    if sess_id not in blocks and type_rec == "M":
        blocks[sess_id] = { 'sess' : SessionMeta(sess_id, line_s[1],line_s[3],line_s[4]), 'events' : list() }
    else:
        if type_rec == "Q":
            o = Query(line_s[1], line_s[3], line_s[4], line_s[4:])
        if type_rec == "C":
            o = Click(line_s[1], line_s[3], line_s[4])
        if type_rec == "S":
            o = Switch(line_s[1])
        blocks[sess_id]['events'].append(o) 
print repr(blocks)       
