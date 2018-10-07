



class Operation(object):
    def do(self, d):
        raise NotImplementedError

    def __call__(self, d):
        return self.do(d)

    def __repr__(self):
        return self.__class__.__name__
