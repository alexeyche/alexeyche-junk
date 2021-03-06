
from collections import defaultdict, OrderedDict
import types
import imp
import pprint
from copy import deepcopy

try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

class Config(defaultdict):
    def __init__(self, *args, **kwargs):
        if len(args) == 0 and len(kwargs) == 0:
            super(Config, self).__init__(Config)
        else:
            super(Config, self).__init__(*args, **kwargs) # for copy

    def __getattr__(self, name):
        if self.get("__frozen", False) and name not in self:
            raise Exception("Failed to find config field: {}".format(name))
        return self[name]

    def __setattr__(self, name, value):
        self[name] = value

    def __delattr__(self, name):
        del self[name]

    def as_frozen(self):
        self["__frozen"] = True
        return self

    def __str__(self):
        return pprint.pformat(dictionarize(self), width=1)  # pprint doesn't work with defaultdict's


    def __repr__(self):
        return "ConfigInstance({})".format(str(self))


    @staticmethod
    def deep_copy(c):
        nc = Config()
        for k, v in c.items():
            if isinstance(v, Config):
                v = Config.deep_copy(v)
            nc[k] = v
        return nc

    def copy(self, **kwargs):
        c = Config.deep_copy(self)
        for k, v in kwargs.items():
            c[k] = v
        return c

    @staticmethod
    def from_dictionary(d):
        dst = Config()

        for k, v in d.items():
            if type(v) is dict or type(v) is OrderedDict:
                v = Config.from_dictionary(v)
                setattr(dst, k, v)
            elif type(v) is list:
                dst_list = []
                for vv in v:
                    if type(vv) is dict or type(v) is OrderedDict:
                        vv = Config.from_dictionary(vv)
                    dst_list.append(vv)

                setattr(dst, k, dst_list)
            else:
                setattr(dst, k, v)
        return dst

    @staticmethod
    def merge_instances(*c):
        res = Config()
        for cc in c:
            res.update(cc)
        return res

def dictionarize(defdict):
    work_defdict = defdict.copy()
    for k, v in work_defdict.items():
        if isinstance(v, Config):
            work_defdict[k] = dictionarize(v)
    return dict(work_defdict)


def read_config(config_file):
    config_module = imp.load_source('config', config_file)
    configs = [
        config_module.__dict__.get(a)
        for a in dir(config_module) if isinstance(config_module.__dict__.get(a), Config)
    ]
    if len(configs) == 0:
        raise Exception("Failed to find any instances of Config in {}".format(config_file))
    if len(configs) > 1:
        raise Exception("Found too many instances of Config in {}".format(config_file))

    return configs[0]



