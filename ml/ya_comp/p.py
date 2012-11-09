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


def read_in_chunks(file_object, chunk_size=1024):
    """Lazy function (generator) to read a file piece by piece.
    Default chunk size: 1k."""
    while True:
        data = file_object.read(chunk_size)
        if not data:
            break
        yield data


def stream_lines(file_object):
    while True:
      line = file_object.readline()
      if not line:
        file_object.close()
        break
      yield line

    

def parse_line(line):
    line_w = line.rstrip('\n')
    line_s = line_w.split('\t')
    sess_id = line_s[0]
    type_rec = line_s[2]
    if type_rec == "M":
        o = SessionMeta(sess_id, line_s[1],line_s[3],line_s[4])
    if type_rec == "Q":
        o = Query(line_s[1], line_s[3], line_s[4], line_s[4:])
    if type_rec == "C":
        o = Click(line_s[1], line_s[3], line_s[4])
    if type_rec == "S":
        o = Switch(line_s[1])
    return o    


ClickForEachSession = 0
QueryForEachSession = 0

def write_stat(file_object):
    str = "%s\t%s\n" % (ClickForEachSession, QueryForEachSession,)
    file_object.write(str)

f = open('dataset/train','r')
out = open('stat','w')
out.write("C\tQ\n")

FirstTime = True
for line in stream_lines(f):
    o = parse_line(line) 
#    import pdb; pdb.set_trace()
    if o.__class__ is SessionMeta:
        if FirstTime:
            FirstTime=False
        else:
            write_stat(out)
        ClickForEachSession = 0
        QueryForEachSession = 0
        continue
    if o.__class__ is Click:
        ClickForEachSession += 1
    if o.__class__ is Query:
        QueryForEachSession += 1

out.close()
f.close()
