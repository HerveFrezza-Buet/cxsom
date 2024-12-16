import os
import sys
import pycxsom as cx
import numpy as np
import sample

if len(sys.argv) < 5:
    print()
    print('Usage:')
    print(f'  {sys.argv[0]} <root_dir> <hostname> <port> <shape>')
    print()
    sys.exit(0)
    
root_dir = sys.argv[1]
hostname = sys.argv[2]
port     = int(sys.argv[3])
mode     = sys.argv[4]

x_var_path = cx.variable.path_from(root_dir, 'in', 'X')
y_var_path = cx.variable.path_from(root_dir, 'in', 'Y')

# We feed the inputs buffers... but not beyond.
with cx.variable.Realize(x_var_path) as X:
    with cx.variable.Realize(y_var_path) as Y:
        r = X.time_range()
        if r is not None:
            tmax = r[1]
            if tmax >= X.file_size - 1:
                nb_to_write = 0
                print()
                print('The input buffers are full, no further feeding is performed.')
                print()
            else:
                nb_to_write = X.file_size - 1 - tmax
        else:
            nb_to_write = X.file_size
        for _ in range(nb_to_write):
            x, y = sample.get(np.random.random(), mode)
            X += x
            Y += y
            
print('Sending "ping" to {}:{}'.format(hostname, port))
if cx.client.ping(hostname, port):
    print('Something went wrong.')
