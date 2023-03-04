import struct
import numpy as np

from . import error

SIZE_OF_DOUBLE = 8

class Type:
    def __init__(self):
        pass
    def __str__(self):
        return 'AbstractType'
    
class Scalar(Type):
    def __init__(self):
        pass
    def __str__(self):
        return 'Scalar'
    def length(self):
        return SIZE_OF_DOUBLE
    def shape(self):
        return (1,)
    def pack(self, value):
        return struct.pack('d', value)
    def unpack(self, buf):
        return struct.unpack('d', buf)[0]
        
class Pos1D(Type):
    def __init__(self):
        pass
    def __str__(self):
        return 'Pos1D'
    def length(self):
        return SIZE_OF_DOUBLE
    def shape(self):
        return (1,)
    def pack(self, value):
        return struct.pack('d', value)
    def unpack(self, buf):
        return struct.unpack('d', buf)[0]

class Values(Type):
    def __init__(self):
        pass
    def pack(self, value):
        return value.tobytes()
    def unpack(self, buf):
        return np.frombuffer(buf, dtype=float).reshape(self.shape())
    
class Pos2D(Values):
    def __init__(self):
        pass
    def __str__(self):
        return 'Pos2D'
    def length(self):
        return 2*SIZE_OF_DOUBLE
    def shape(self):
        return (2,)

class Array(Values):
    def __init__(self, dim):
        self.dim = dim
    def __str__(self):
        return 'Array={}'.format(self.dim)
    def length(self):
        return self.dim*SIZE_OF_DOUBLE
    def shape(self):
        return (self.dim,)

class Map(Values):
    def __init__(self, side, content):
        self.side    = side
        self.content = content

class Map1D(Map):
    def __init__(self, side, content):
        Map.__init__(self, side, content)
    def __str__(self):
        return 'Map1D<{}>={}'.format(self.content, self.side)
    def length(self):
        return self.side * self.content.length()
    def shape(self):
        if self.content.shape()[0] == 1:
            return (self.side,)
        else:
            return (self.side, self.content.shape()[0])

class Map2D(Map):
    def __init__(self, side, content):
        Map.__init__(self, side, content)
    def __str__(self):
        return 'Map2D<{}>={}'.format(self.content, self.side)
    def length(self):
        return self.side * self.side * self.content.length()
    def shape(self):
        if self.content.shape()[0] == 1:
            return (self.side, self.side)
        else:
            return (self.side, self.side, self.content.shape()[0])
    
def make(name):
    """
    name : a string describying a type, e.g 'Map1D<Pos2D>=30'
    returns : a type instance. Here Map2D(30, Pos2D())
    """
    if len(name) == 0 :
        raise error.Parse("make('{}')".format(name), 'Empty type description')
    
    return parse_TYPE(name)[0]

# This is the grammar

# TYPE    :=   CONTENT 
#            | MAP
#
# CONTENT :=   'Scalar'
#            | POS
#            | ARRAY
#
# POS     := 'Pos' DIM
#
# ARRAY   := 'Array' SIZE
#
# MAP     := 'Map' DIM '<' CONTENT '>' SIZE
#
# SIZE    := '=' <int>
#
# DIM     :=   '1D' 
#            | '2D'

def parse_check_next(charstream, tag):
    if len(charstream) < len(tag) or charstream[:len(tag)] != tag:
        raise error.Parse("parse_check_next('{}', ...)".format(charstream), "'{}' expected.".format(tag))
    return charstream[len(tag):]
        
def parse_TYPE(charstream):
    if charstream[0] == 'M':
        return parse_MAP(charstream)
    else:
        return parse_CONTENT(charstream)

def parse_CONTENT(charstream):
    if charstream[0] == 'S':
        charstream = parse_check_next(charstream, 'Scalar')
        return (Scalar(), charstream)
    if charstream[0] == 'P':
        return parse_POS(charstream)
    if charstream[0] == 'A':
        return parse_ARRAY(charstream)
    
def parse_POS(charstream):
    charstream      = parse_check_next(charstream, 'Pos')
    dim, charstream = parse_DIM(charstream)
    if dim == 1:
        return (Pos1D(), charstream)
    if dim == 2:
        return (Pos2D(), charstream)
    
def parse_ARRAY(charstream):
    charstream       = parse_check_next(charstream, 'Array')
    size, charstream = parse_SIZE(charstream)
    return (Array(size), charstream)
    
def parse_MAP(charstream):
    charstream       = parse_check_next(charstream, 'Map')
    dim, charstream  = parse_DIM(charstream)
    charstream       = parse_check_next(charstream, '<')
    c, charstream    = parse_CONTENT(charstream)
    charstream       = parse_check_next(charstream, '>')
    size, charstream = parse_SIZE(charstream)
    if dim == 1:
        return (Map1D(size, c), charstream)
    if dim == 2:
        return (Map2D(size, c), charstream)
    
def parse_SIZE(charstream):
    charstream = parse_check_next(charstream, '=')
    i = 0
    while i < len(charstream) and charstream[i].isdigit():
        i += 1
    if i == 0:
        raise error.Parse("parse_SIZE(='{}')".format(charstream), 'integer expected after =.')
    return (int(charstream[:i]), charstream[i:])
    
def parse_DIM(charstream):
    try:
        charstream = parse_check_next(charstream, '1D')
        return (1, charstream)
    except error.Parse as err:
        try:
            charstream = parse_check_next(charstream, '2D')
            return (2, charstream)
        except error.Parse as err:
            raise error.Parse("parse_DIM('{}')".format(charstream), 'This is not 1D or 2D.')
        
            
        


