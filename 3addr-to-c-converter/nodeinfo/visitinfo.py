
wave = 0

def new_visit():
    global wave
    wave += 1

class VisitInfo:
    _wave = 0

    @apply
    def visited():
        def fset(self, value):
            if value is True:
                self._wave = wave
        def fget(self):
            return (self._wave == wave)
        return property(**locals())

    @property
    def neighbors(self):
        raise NotImplementedError("VisitInfo.neighbours")
