#!/usr/bin/python -i

import time
import Queue
import threading

# system

def enum(**enums):
    return type('Enum', (), enums)

# env

colors = enum(Red = 255)

# mind

class Signal():
    def __init__(self):
        pass

class EyeSignalObject():
    def __init__(self,inf):
        self.inf = inf

class EyeSignal(Signal):
    def __init__(self,eye_object):
        self.obj = eye_object

apple = EyeSignalObject({'size':4,'color': colors.Red })
apple_sign = EyeSignal(apple)

class Mind():
    def __init__(self):
        self.input = Queue.Queue()        
        self.map = {}
    def start(self):
        while(True):
            while (not self.input.empty()):
                signal = self.input.get()
                self.map[id(signal.obj)] = signal.obj.inf        
            
            time.sleep(1)            

    def get_signal(self,signal):
        self.input.put(signal)

m = Mind()


t = threading.Thread(target = m.start, args = () )
t.start()

m.get_signal(apple_sign)


