import os
import sys
import time
import pycxsom as cx


if len(sys.argv) != 4:
    print()
    print('Usage:')
    print('  {} <root_dir> <timeline> <varname>'.format(sys.argv[0]))
    print()
    sys.exit(0)
    
root_dir = sys.argv[1]
timeline = sys.argv[2]
varname  = sys.argv[3]

path = str(cx.variable.path_from(root_dir, timeline, varname))
file_size = None
while True:
    time.sleep(1.0)
    s = os.stat(path).st_size
    if s == file_size:
        break
    file_size = s

    
