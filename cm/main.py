
import time

from node import node
from graph import graph

from flask import Flask
from flask import render_template
from flask import request, send_from_directory

@node()
def fun0():
    time.sleep(1.0)
    print "Hello from fun0"

@node()
def fun00():
    time.sleep(1.0)
    print "Hello from fun00"



@node(depends = (fun0, ))
def fun1():
    time.sleep(1.0)
    print "Hello from run1"


@node(depends = (fun1, ))
def fun2():
    time.sleep(1.0)
    print "Hello from run2"


@node(depends = (fun1, ))
def fun3():
    time.sleep(1.0)
    print "Hello from run3"



@node(depends = (fun00, ))
def fun4():
    time.sleep(1.0)
    print "Hello from run4"


from flask import make_response
from functools import wraps, update_wrapper
from datetime import datetime

def nocache(view):
    @wraps(view)
    def no_cache(*args, **kwargs):
        response = make_response(view(*args, **kwargs))
        response.headers['Last-Modified'] = datetime.now()
        response.headers['Cache-Control'] = 'no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0'
        response.headers['Pragma'] = 'no-cache'
        response.headers['Expires'] = '-1'
        return response
        
    return update_wrapper(no_cache, view)



graph.resfresh()


app = Flask("graph")
app.config["CACHE_TYPE"] = "null"

@app.route('/js/<path:path>')
def send_js(path):
    return send_from_directory('js', path)


@app.route("/")
@nocache
def index():
    node_data = []
    edge_data = []
    for n in graph.get_order():
        node_data.append({"id": n.get_idx(), "label": n.get_name(), 'group': n.get_state()})
        for v in n.get_vertices():
            edge_data.append({"from": n.get_idx(), "to": v.get_idx()})

    return render_template('index.html', node_data = node_data, edge_data = edge_data)




if __name__ == "__main__":
    app.run()


