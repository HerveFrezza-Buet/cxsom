import os
import sys
import pycxsom as cx
import numpy as np
import sample

if len(sys.argv) < 5:
    print()
    print('Usage:')
    print('  {} <root_dir> <hostname> <port> <frozen-timeline-prefix>'.format(sys.argv[0]))
    print()
    sys.exit(0)
    
root_dir = sys.argv[1]
hostname = sys.argv[2]
port     = int(sys.argv[3])
timeline = sys.argv[4]

x_var_path = cx.variable.path_from(root_dir, timeline + '-in', 'X')
y_var_path = cx.variable.path_from(root_dir, timeline + '-in', 'Y')
u_var_path = cx.variable.path_from(root_dir, timeline + '-in', 'U')

with cx.variable.Realize(x_var_path) as X:
    with cx.variable.Realize(y_var_path) as Y:
        with cx.variable.Realize(u_var_path) as U:
            r = U.time_range()
            if r is not None:
                tmax = r[1]
                if tmax >= U.file_size:
                    nb_to_write = 0
                else:
                    nb_to_write = U.file_size - tmax
            else:
                nb_to_write = U.file_size
            if nb_to_write > 0:
                idx = np.flip(U.file_size - 1 - np.arange(nb_to_write)) / U.file_size
                for u in idx:
                    x, y = sample.get(u)
                    X += x
                    Y += y
                    U += u
        
print('Sending "ping" to {}:{}'.format(hostname, port))
if cx.client.ping(hostname, port):
    print('Something went wrong.')
