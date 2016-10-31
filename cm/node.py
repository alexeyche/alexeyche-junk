
from functools import wraps

from graph import graph

class TargetState(object):
    IDLE = "idle"
    RUNNING = "running"
    FINISHED = "finished"
    FAILED = "failed"


class node(object):
    def __init__(self, depends = ()):
        self.vertices = []
        self.callback = None

        self.state = TargetState.IDLE 
        for dep in depends:
            dep.node.add_vertex(self)
        
        self.root_node = len(depends) == 0
        self.idx = None

    def __call__(self, func):
        @wraps(func)        
        def wrappee(*args, **kwargs):
            self.change_state(TargetState.RUNNING)
            try:
                ret = func(*args,**kwargs)
                self.change_state(TargetState.FINISHED)
                return ret
            except:
                self.change_state(TargetState.FAILED)
        
        if self.root_node:
        	graph.registry_root(self)
            
        self.callback = wrappee
        wrappee.node = self
        return wrappee

    def set_idx(self, idx):
        self.idx = idx
    
    def get_idx(self):
        return self.idx

    def change_state(self, state):
        print "Changing state of {} to {}".format(self, state)
        self.state = state


   	def __str__(self):
   		return str(self)

    def __repr__(self):
        return "{}Node({}, state {}{})".format(
            "" if not self.root_node else "Root", 
            self.callback.__name__, 
            self.state, 
            "" if self.idx is None else ", id: {}".format(self.idx))
    
    def add_vertex(self, v):
        self.vertices.append(v)

    def get_vertices(self):
        return self.vertices

    def get_name(self):
        return self.callback.__name__

    def get_state(self):
        return self.state
