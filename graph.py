
import time
from functools import wraps

class Registry(object):
    def __init__(self):
        self.root_nodes = []

    def registry_root(self, root):
        self.root_nodes.append(root)


registry = Registry()



class TargetState(object):
    IDLE = "idle"
    RUNNING = "running"
    FINISHED = "finished"
    FAILED = "failed"


class Target(object):
    def __init__(self, *args, **kwargs):
        self.vertices = []
        self.callback = None

        self.state = TargetState.IDLE 

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

            
        self.callback = wrappee
        wrappee.node = self
        return wrappee


    def change_state(self, state):
        print "Changing state of {} to {}".format(self, state)
        self.state = state


    def __repr__(self):
        return "{}({} vertices, {})".format(self, len(self.vertices), self.state)

    def add_vertex(self, v):
        self.vertices.append(v)



class root_node(Target):
    def __init__(self, *args, **kwargs):
        super(root_node, self).__init__(*args, **kwargs)

    def __call__(self, func):
        registry.registry_root(self)
        return super(root_node, self).__call__(func)

    def __str__(self):
        return "RootNode"


class node(Target):
    def __init__(self, depends, *args, **kwargs):
        super(node, self).__init__(*args, **kwargs)
        for dep in depends:
            dep.node.add_vertex(self)


    def __str__(self):
        return "Node"

    def __call__(self, func):
        return super(node, self).__call__(func)




@root_node()
def fun0():
    time.sleep(1.0)
    print "Hello from fun0"

@root_node()
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







def traverse(r):
    r.callback()
    for d in r.vertices:
        traverse(d)


for n in registry.root_nodes:
    traverse(n)






