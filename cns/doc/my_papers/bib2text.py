#!/usr/bin/env python
import sys


class Ref(object):
    author = ""
    title = ""
    journal = ""
    year = ""
    volume = ""
    number = ""
    pages = ""
    publisher = ""
    def __str__(self):
        out = "%s \"%s\", %s" % (self.author, self.title, self.year)
        if self.journal:
            out += ", %s" % self.journal
        if self.publisher:
            out += ", %s" % self.publisher
        if self.volume:
            out += ", %s" % self.volume
        if self.number:
            out += ", %s" % self.number
        if self.pages:
            out += ", %s" % self.pages
                     

        return out

r = None
for l in sys.stdin:
    if l.strip().startswith("@"):
        if r:
            print r
        r = Ref()        
    l_spl = l.split("=")
    if len(l_spl) == 2:
        r.__dict__[l_spl[0].strip()] = l_spl[1].strip("\n,{} ")
