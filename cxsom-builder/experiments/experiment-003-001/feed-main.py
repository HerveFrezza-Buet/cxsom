import os
import sys
import pycxsom as cx
import numpy as np
import sample

if len(sys.argv) < 4:
    print()
    print('Usage:')
    print('  {} <root_dir> <hostname> <port>'.format(sys.argv[0]))
    print()
    sys.exit(0)
    
root_dir = sys.argv[1]
hostname = sys.argv[2]
port     = int(sys.argv[3])

x_var_path = cx.variable.path_from(root_dir, 'in', 'X')
y_var_path = cx.variable.path_from(root_dir, 'in', 'Y')

with cx.variable.Realize(x_var_path) as X:
    with cx.variable.Realize(y_var_path) as Y:
        r = X.time_range()
        if r is not None:
            tmax = r[1]
            if tmax >= X.file_size:
                nb_to_write = 0
            else:
                nb_to_write = X.file_size - tmax
        else:
            nb_to_write = X.file_size
        for _ in range(nb_to_write):
            x, y = sample.get(np.random.random())
            X += x
            Y += y
            
print('Sending "ping" to {}:{}'.format(hostname, port))
if cx.client.ping(hostname, port):
    print('Something went wrong.')
