import os
import os.path
import struct
import time

from . import typing
from . import error

UINT_LENGTH                  = 8
LENGTH_TYPE_IN_FILE          = 64
LENGTH_CACHE_SIZE_IN_FILE    = UINT_LENGTH
LENGTH_FILE_SIZE_IN_FILE     = UINT_LENGTH
LENGTH_HIGHEST_TIME_IN_FILE  = UINT_LENGTH
LENGTH_NEXT_FREE_POS_IN_FILE = UINT_LENGTH

OFFSET_TYPE_IN_FILE          = 0
OFFSET_CACHE_SIZE_IN_FILE    = OFFSET_TYPE_IN_FILE          + LENGTH_TYPE_IN_FILE         
OFFSET_FILE_SIZE_IN_FILE     = OFFSET_CACHE_SIZE_IN_FILE    + LENGTH_CACHE_SIZE_IN_FILE   
OFFSET_HIGHEST_TIME_IN_FILE  = OFFSET_FILE_SIZE_IN_FILE     + LENGTH_FILE_SIZE_IN_FILE    
OFFSET_NEXT_FREE_POS_IN_FILE = OFFSET_HIGHEST_TIME_IN_FILE  + LENGTH_HIGHEST_TIME_IN_FILE 
OFFSET_HEADER_IN_FILE        = OFFSET_NEXT_FREE_POS_IN_FILE + LENGTH_NEXT_FREE_POS_IN_FILE

no_time = struct.unpack('Q', b'\xff\xff\xff\xff\xff\xff\xff\xff')[0]

def path_from(root_dir, timeline, varname):
    varname += '.var'
    names = varname.split('\\')
    return os.path.join(root_dir, timeline, os.path.join(*names))

def names_from(varpath):
    """
    varname : Should be '<rootdir>/<timeline>/<varname>.var'
    returns : (root_dir, timeline, varname), or None
    """
    elems = varpath.split('/')
    if len(elems) < 3:
        return None
    root_dir = elems[0]
    timeline = elems[1]
    varname = elems[2]
    for name in elems[3:]:
        varname = varname + '/' + name
    varname = varname.split('.')[0]
    return (root_dir, timeline, varname)
    
    
    
    
class Realize:
    def __init__(self, path, t = None, cache_size = None, file_size = None):
        """
        path : the file path (.var) of the variable.
        type : If not None, it is used to create an empty variable if the .var file does not exist yet.
        cache_size : If not none, it sets the cache size (used at server side) if the .var file does not exist yet.
        file_size : If not none, this sets the file buffer size if the .var file does not exist yet.
        """
        self.path = path
        if(os.path.exists(path)):
            self.datafile = open(path, 'rb')
            s = self.datafile.read(LENGTH_TYPE_IN_FILE)
            for idx, b in enumerate(s):
                if b == 10 :
                    break
            self.datatype = typing.make(s[:idx].decode('ascii'))
            self.datafile.close()
            if (t is not None) and (str(t) != str(self.datatype)):
                raise error.Typing(t, "Existing variable {} as already a type {}".format(self.path, self.datatype))
            if cache_size is not None:
                self.datafile = open(self.path, 'rb+')
                self.datafile.seek(OFFSET_CACHE_SIZE_IN_FILE, os.SEEK_SET)
                self.datafile.write(struct.pack('Q', cache_size))
                self.datafile.close()
                
        else:
            if cache_size == None:
                raise error.Unspecified('cache_size for {}'.format(self.path))
            if file_size == None:
                raise error.Unspecified('file_size for {}'.format(self.path))
            if t == None:
                raise error.Unspecified('type for {}'.format(self.path))
            dir_path = os.path.dirname(path)
            if not os.path.exists(dir_path):
                os.makedirs(os.path.dirname(path))
            self.datafile = open(path, 'wb')
            self.datatype = t
            self.datafile.write(bytearray(str(self.datatype)+"\n", 'ascii'))
            pad = LENGTH_TYPE_IN_FILE - (len(str(self.datatype))+1)
            self.datafile.write(bytes(pad))
            self.datafile.write(struct.pack('Q', cache_size))
            self.datafile.write(struct.pack('Q', file_size))
            self.datafile.write(bytes(b'\xff\xff\xff\xff\xff\xff\xff\xff'))
            self.datafile.write(bytes(b'\x00\x00\x00\x00\x00\x00\x00\x00'))
            self.datafile.close()
        self.datafile = None 


        
    def private_update(self):
        self.datafile.seek(OFFSET_CACHE_SIZE_IN_FILE, os.SEEK_SET)
        self.cache_size    = struct.unpack('Q', self.datafile.read(UINT_LENGTH))[0]
        self.datafile.seek(OFFSET_FILE_SIZE_IN_FILE, os.SEEK_SET)
        self.file_size     = struct.unpack('Q', self.datafile.read(UINT_LENGTH))[0]
        self.datafile.seek(OFFSET_HIGHEST_TIME_IN_FILE, os.SEEK_SET)
        self.highest_time  = struct.unpack('Q', self.datafile.read(UINT_LENGTH))[0]
        self.datafile.seek(OFFSET_NEXT_FREE_POS_IN_FILE, os.SEEK_SET)
        self.next_free_pos = struct.unpack('Q', self.datafile.read(UINT_LENGTH))[0]
        
    def open(self):
        self.close()
        self.datafile      = open(self.path, 'rb+')
        self.data_length   = self.datatype.length()
        self.slot_length   = self.data_length + 1
        self.private_update()
        
    def close(self):
        if self.datafile != None:
            self.datafile.close()
            self.datafile = None

    def __enter__(self):
        self.open()
        return self
    
    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.close()
        if exc_type:
            print(f'exc_type: {exc_type}')
            print(f'exc_value: {exc_value}')
            print(f'exc_traceback: {exc_traceback}')

    def update(self):
        """This forces a re-read of the information stored in the header. It
        can be usefull if somebody else changes the file while it is
        beeing handled.
        """
        self.close()
        self.datafile = open(self.path, 'rb+')
        self.private_update()
        
    def time_range(self):
        """
        if the variable is opened, it returns the range of values in the buffer as a tuple [tmin, tmax]. 
        It may return None is no values are stored in the file.
        """
        if self.file_size == 0 or self.highest_time == no_time:
            return None
        if self.highest_time < self.file_size:
            return (0, self.highest_time)
        return (self.highest_time - self.file_size + 1, self.highest_time)

    def private_index_of(self, at):
        if at >= 0:
            return at
        else:
            r = self.time_range()
            if r :
                att = r[1] + 1 + at
                if att >= 0:
                    return att
            raise error.Index(at)
            

    def private_next_pos(self, at):
        res = at + 1
        if res == self.file_size:
            return 0
        return res

    def private_is_past_in_buffer(self, highest_time_minus_at):
        return highest_time_minus_at < self.file_size
            
    def private_pos_in_past(self, highest_time_minus_at):
        if highest_time_minus_at < self.next_free_pos:
            return self.next_free_pos - highest_time_minus_at - 1;
        return self.file_size + self.next_free_pos - highest_time_minus_at - 1;

    def private_seek(self, at):
        self.private_seek_htma(self.highest_time - at)
        
    def private_seek_htma(self, highest_time_minus_at):
        self.datafile.seek(OFFSET_HEADER_IN_FILE + self.private_pos_in_past(highest_time_minus_at) * self.slot_length, os.SEEK_SET)
        

    def __getitem__(self, at):
        """
        Retrieves the variable content. It may raise an Busy exception if the 
        variable is not set yet, or a Forgotten exception is you try to read 
        in the past, outside the file history.
        """
        at = self.private_index_of(at)
        if self.file_size == 0:
            raise error.Forgotten()
        if self.highest_time == no_time:
            raise error.Busy()
        if at > self.highest_time:
            raise error.Busy()
        
        highest_time_minus_at = self.highest_time - at
        if self.private_is_past_in_buffer(highest_time_minus_at):
            self.private_seek_htma(highest_time_minus_at)
            if struct.unpack('?', self.datafile.read(1))[0]:
                return self.datatype.unpack(self.datafile.read(self.data_length))
            else:
                raise error.Busy()
        else:
            raise error.Forgotten()

    def __setitem__(self, at, value):
        """
        Sets the variable content. It may raise an Ready exception if the 
        variable was already set, or a Forgotten exception is you try to set 
        in the past, outside the file history.
        """
        at = self.private_index_of(at)
        
        if self.file_size == 0:
            raise error.Forgotten()
        
        if self.highest_time == no_time:
            self.datafile.seek(OFFSET_HEADER_IN_FILE, os.SEEK_SET)
            null_slot = b'\0' * self.slot_length
            if at > self.file_size:
                for i in range(self.file_size):
                    self.datafile.write(null_slot)
                self.datafile.seek(OFFSET_HEADER_IN_FILE, os.SEEK_SET)
                self.next_free_pos = self.private_next_pos(0)
            else:
                for i in range(at):
                    self.datafile.write(null_slot)
                self.next_free_pos = self.private_next_pos(at)
            self.datafile.write(struct.pack('?', True))
            self.datafile.write(self.datatype.pack(value))
            self.highest_time = at
            self.datafile.seek(OFFSET_HIGHEST_TIME_IN_FILE, os.SEEK_SET)
            self.datafile.write(struct.pack('Q', at))
            self.datafile.seek(OFFSET_NEXT_FREE_POS_IN_FILE, os.SEEK_SET)
            self.datafile.write(struct.pack('Q', self.next_free_pos))
            self.datafile.flush()
            return

        if at <= self.highest_time:
            htma = self.highest_time - at
            if self.private_is_past_in_buffer(htma):
                self.private_seek_htma(htma)
                pos = self.datafile.tell()
                if struct.unpack('?', self.datafile.read(1))[0]:
                    raise error.Ready()
                self.datafile.seek(pos, os.SEEK_SET)
                self.datafile.write(struct.pack('?', True))
                self.datafile.write(self.datatype.pack(value))
                self.datafile.flush()
                return
            else:
                raise error.Forgotten()

        nb_zeros  = at - 1 - self.highest_time
        null_slot = b'\0' * self.slot_length
        if nb_zeros > self.file_size:
            self.datafile.seek(OFFSET_HEADER_IN_FILE, os.SEEK_SET)
            for i in range(self.file_size):
                self.datafile.write(null_slot)
            self.datafile.seek(OFFSET_HEADER_IN_FILE, os.SEEK_SET)
            self.next_free_pos = self.private_next_pos(0)
        else:
            data_pos      = self.next_free_pos + nb_zeros
            upper_bound   = min(self.file_size, data_pos)
            nb_to_the_end = upper_bound - self.next_free_pos
            self.datafile.seek(OFFSET_HEADER_IN_FILE + self.next_free_pos * self.slot_length, os.SEEK_SET)
            for i in range(nb_to_the_end):
                self.datafile.write(null_slot)
            if data_pos >= self.file_size:
                data_pos = data_pos - self.file_size
                self.datafile.seek(OFFSET_HEADER_IN_FILE, os.SEEK_SET)
                for i in range(data_pos):
                    self.datafile.write(null_slot)
            self.next_free_pos = self.private_next_pos(data_pos)
        self.datafile.write(struct.pack('?', True))
        self.datafile.write(self.datatype.pack(value))
        self.highest_time = at
        self.datafile.seek(OFFSET_HIGHEST_TIME_IN_FILE, os.SEEK_SET)
        self.datafile.write(struct.pack('Q', at))
        self.datafile.seek(OFFSET_NEXT_FREE_POS_IN_FILE, os.SEEK_SET)
        self.datafile.write(struct.pack('Q', self.next_free_pos))
        self.datafile.flush()
        return
            
        
    def __iadd__(self, value):
        r = self.time_range()
        if r:
            self[r[1]+1] = value
        else:
            self[0]      = value
        return self

    def sync_init(self):
        self.update()
        self.sync0 = self.time_range()
        
    def wait_next(self, sleep_duration=.1):
        dd = self.sync0
        while self.sync0 == dd:
            time.sleep(sleep_duration)
            self.update()
            dd = self.time_range()

    def listen(self, func, sleep_duration=.1):
        dd = self.sync0
        while True:
            while self.sync0 == dd:
                time.sleep(sleep_duration)
                self.update()
                dd = self.time_range()
            self.sync0 = dd
            func(self)


def data_range_full(varpath):
    data = Realize(varpath)
    data.open()
    r = data.time_range()
    if r == None:
        return
    for at in range(r[0], r[1]+1):
        v = None
        try:
            v = data[at]
        except error.Busy:
            pass
        yield (at, v)
    data.close()
    return

def data_range_lasts(varpath, nb):
    if nb <= 0:
        return
    data = Realize(varpath)
    data.open()
    r = data.time_range()
    if r == None:
        return
    inf = max(r[1] + 1 - nb, r[0])
    for at in range(inf, r[1]+1):
        v = None
        try:
            v = data[at]
        except error.Busy:
            pass
        yield (at, v)
    data.close()
    return

def data_range_lasts_before(varpath, nb, at):
    if nb <= 0:
        return
    data = Realize(varpath)
    data.open()
    r = data.time_range()
    if r == None:
        return
    sup = min(r[1], at)
    inf = max(at + 1 - nb, r[0])
    for at in range(inf, sup + 1):
        v = None
        try:
            v = data[at]
        except error.Busy:
            pass
        yield (at, v)
    data.close()
    return
        
    















