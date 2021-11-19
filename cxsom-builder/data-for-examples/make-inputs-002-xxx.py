import os
import sys
import pycxsom as cx
import numpy as np

INPUT_BUNCH_SIZE = 5000

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
          
with cx.variable.Realize(x_var_path, cx.typing.make('Scalar'), 1, INPUT_BUNCH_SIZE) as X:
    with cx.variable.Realize(y_var_path, cx.typing.make('Scalar'), 1, INPUT_BUNCH_SIZE) as Y:
        for i in range(INPUT_BUNCH_SIZE):
            t = np.random.random()*2*np.pi
            X += .5 * (np.cos(t) + 1)
            Y += .5 * (np.sin(t) + 1)

print('Sending "ping" to {}:{}'.format(hostname, port))
if cx.client.ping(hostname, port):
    print('Something went wrong.')

            
          
          
          

