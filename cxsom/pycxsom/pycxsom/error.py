
class Any(Exception):
    def __init__(self, kind, where, what):
        self.message = "{} : {} : {}".format(kind, where, what)
    def __str__(self):
        return self.message
        
class Typing(Any):
    def __init__(self, t, where):
        Any.__init__(self, "Type error", where, "an issue is raised by type {}".format(t))

class Parse(Any):
    def __init__(self, where, what):
        Any.__init__(self, "Parse error", where, what)

class Busy(Any):
    def __init__(self):
        Any.__init__(self, "Busy slot", "", "")

class Ready(Any):
    def __init__(self):
        Any.__init__(self, "Ready slot", "", "")

class Forgotten(Any):
    def __init__(self):
        Any.__init__(self, "Forgotten slot", "", "")
        
class Unspecified(Any):
    def __init__(self, what):
        Any.__init__(self, "Unspecified value (None)", "", "A value for {} is required".format(what))

class Index(Any):
    def __init__(self, at):
        Any.__init__(self, "Bad index", "", str(at))
