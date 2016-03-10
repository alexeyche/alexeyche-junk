#!/usr/bin/env python


import re
import os
from os.path import join as pj
import argparse
from collections import defaultdict

known_types = {
    "double"    : "double",
    "int"       : "int32",
    "size_t"    : "uint32",
    "ui32"      : "uint32",
    "float"     : "float",
    "string"    : "string",
    "bool"      : "bool",
    "complex<double>" : "TComplex",
    "pair<string, size_t>" : "TStringToUintPair",
    "pair<size_t, size_t>" : "TUintToUintPair",
}



vector_re_str = "(?:TVector|vector|TActVector)+"
vector_re = re.compile("{}<(.*)>".format(vector_re_str))
field_re = re.compile(".*[ ]*({})[\W]+(?!__)([a-zA-Z]+);[ ]*$".format(
	"|".join(known_types.keys()) #+ [ "{}<{}>".format(VECTOR_RE_STR, t) for t in KNOWN_TYPES.keys() ])
))

struct_re = re.compile(".*[ ]*struct[ ]*(T[a-zA-Z0-9_]+).*")
cpp_re = re.compile("(.*)(\.h|\.cpp)$")

ignore_re = re.compile(".*dnn/contrib.*")

class TStruct(object):
	def __init__(self, name):
		self.name = name
		self.fields = []

	def __str__(self):
		return "{}({})".format(self.name, ", ".join([ "{} {}".format(f[0], f[1]) for f in self.fields ]))

def GenerateProtos(structures_to_file, package, dst, imports):
    for fname, structures in structures_to_file.iteritems():
    	dst_file = "{}.proto".format(fname.split(".")[0])
        with open(os.path.join(dst, dst_file), 'w') as f_ptr:
            f_ptr.write("package %s;\n" % package)
            f_ptr.write("\n")
            for imp in imports:
                f_ptr.write("import \"{}\";\n".format(imp))
                f_ptr.write("\n")
            
            for structure in structures:
	            f_ptr.write("message %s {\n" % structure.name)
	            for f in structure.fields:
	            	i = 1
	                if known_types.get(f[0]) is None:
	                    m = vector_re.match(f[0])
	                    if m is None:
	                        raise Exception("Can't match {}".format(f[0]))
	                    f_ptr.write("    repeated %s %s = %s;\n" % (known_types[ m.group(1) ], f[1], str(i)))
	                else:
	                    f_ptr.write("    required %s %s = %s;\n" % (known_types[ f[0] ], f[1], str(i)))
	                i += 1
	            f_ptr.write("}\n")
	            f_ptr.write("\n")



def ParseStructures(src_dir):
	structures_to_file = defaultdict(list)
		
	for root, dirs, files in os.walk(src_dir):
		if ignore_re.match(root):
			continue

		for f in files:
			cpp_m = cpp_re.match(f)
			if cpp_m:
				open_brackets = 0
				struct = None
				line_num = 0
				for l in open(pj(root, f)):
					l = l.split("//")[0]
					line_num += 1
					struct_m = struct_re.match(l)
					if struct_m:
						struct = TStruct(struct_m.group(1))
						open_brackets = l.count("{")
						continue

					if struct:
						open_brackets += l.count("{")
						open_brackets -= l.count("}")
						if open_brackets == 0:
							structures_to_file[f].append(struct)
							struct = None
							continue
						
						m = field_re.match(l.strip())
						if m:
							struct.fields.append( (m.group(1), m.group(2)) )
	                        continue

	return structures_to_file



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--source-path", help="Path to the sources",
                    type=str, required=True)
    parser.add_argument("-d", "--dest-path", help="Path where to store .proto",
                    type=str, required=True)
    parser.add_argument("-p", "--package", help="Package name, default : %(default)s",
                    type=str, required=False, default="NDnnProtos")
    parser.add_argument("-i", "--imports", help="Put imports to all messages (separated by ;)",
                    type=str, required=False, default=None)
    args = parser.parse_args()
    structures = ParseStructures(args.source_path)
    imports = []
    if args.imports:
        imports = [ v.strip() for v in args.imports.split(";") if v.strip() ]
    GenerateProtos(structures, args.package, args.dest_path, imports)
