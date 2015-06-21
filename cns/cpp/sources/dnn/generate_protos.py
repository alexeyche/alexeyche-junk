#!/usr/bin/env python
import os
import argparse
import re

KNOWN_TYPES = { 
    "double"    : "double", 
    "int"       : "int32", 
    "size_t"    : "uint32", 
    "float"     : "float", 
    "string"    : "string",
    "bool"       : "bool", 
}
VECTOR_RE = re.compile("(?:vector|ActVector)+<([^ ]*?)>")

PROTO_FILE = "generated.proto"

def generateProtos(structures, package, dst):
    with open(os.path.join(dst, PROTO_FILE), 'w') as f_ptr:
        f_ptr.write("package %s;\n" % package)
        f_ptr.write("\n")
        for s in structures:
            f_ptr.write("message %s {\n" % s['name'])
            for i, f in enumerate(s['fields']):
                if KNOWN_TYPES.get(f[0]) is None:
                    m = VECTOR_RE.match(f[0])
                    if m is None:
                        sys.exit(1)
                    f_ptr.write("    repeated %s %s = %s;\n" % (KNOWN_TYPES[ m.group(1) ], f[1], str(i+1)))
                else:
                    f_ptr.write("    required %s %s = %s;\n" % (KNOWN_TYPES[ f[0] ], f[1], str(i+1)))
            f_ptr.write("}\n")
            f_ptr.write("\n")

def parseSources(src):
    structures = list()
    for root, dirs, files in os.walk(src):
        for f in files:
            af = os.path.join(root, f)
            generate_proto = False
            if af.endswith(".cpp") or af.endswith(".h"):
                for l in open(af):
                    l = l.strip()
                    l = l.split("//")[0]
                    if "@GENERATE_PROTO@" in l:
                        generate_proto = True                                                
                        struct = {}
                        curly_counter = 0
                        continue
                    if generate_proto:
                        curly_counter += l.count("{")
                        curly_counter -= l.count("}")
                        if len(struct) == 0:
                            m = re.match("[\W]*(?:class|struct)[\W]+([^ ]+)", l)
                            if not m:
                                raise Exception("Can't parse GENERATE_PROTO class or struct")
                            struct['name'] = m.group(1)                            
                            struct['fields'] = []
                        else:
                            m = re.match(
                                "(%s)[\W]+(?!__)([^ ]*);[\W]*$" % "|".join(
                                    KNOWN_TYPES.keys() + [ "(?:vector|ActVector)+<{}>".format(t) for t in KNOWN_TYPES.keys() ]
                                ), 
                                l
                            )
                            if m and curly_counter == 1:
                                struct['fields'].append( (m.group(1), m.group(2)) )
                                continue
                        if len(struct) > 0 and curly_counter == 0:
                            generate_proto = False
                            structures.append(struct)      
    return structures                        

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--source-path", help="Path to the sources",
                    type=str, required=True)
    parser.add_argument("-d", "--dest-path", help="Path where to store .proto",
                    type=str, required=True)
    parser.add_argument("-p", "--package", help="Package name, default : %(default)s",
                    type=str, required=False, default="Protos")
    args = parser.parse_args() 
    structures = parseSources(args.source_path)
    generateProtos(structures, args.package, args.dest_path)
