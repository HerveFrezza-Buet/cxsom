import os
import sys
import pycxsom as cx
import numpy as np
import time

# When you need to access some variables for reading or writing, it
# may conflict with other processes that do the same. The main one is
# the cxsom-processor. Thanks to skednet, this concurrency can be
# controlled. The cxsom-processor operates a write access to the
# variable files, each time it is able to compute new things. Other
# processes may ask for reading, and even writing the files. Let us
# subscribe to the xrsw skednet protocol (xrsw = multiple readers,
# single writer) to do so.

# Nota : It seems that reading without xrsw is ok... but it is safer
# to have it activated.


if len(sys.argv) != 5:
    print(f'Usage : {sys.argv[0]} [read|write] <second-duration> <hostname> <port>')
    sys.exit(0)

mode     = sys.argv[1]
duration = float(sys.argv[2])
hostname = sys.argv[3]
port     = int(sys.argv[4])

if mode == 'read':
    locker = cx.sked.read(hostname, port)
elif mode == 'write':
    locker = cx.sked.write(hostname, port)
else:
    raise ValueError(f'bad mode argument ({mode}).')

with locker:
    print(f'Starting {mode}...')
    time.sleep(duration)
    print(f'... {mode} done.')



