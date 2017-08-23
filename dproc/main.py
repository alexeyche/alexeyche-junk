

import pydot as pdot
import os

g = pdot.graph_from_dot_file("{home}/prog/alexeyche-junk/dproc/example.dot".format(home=os.environ["HOME"]))