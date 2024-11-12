import os
import sys
import pycxsom as cx
import numpy as np


INPUT_FILE_SIZE = 1000

if len(sys.argv) < 6:
    print()
    print('Usage:')
    print('  {} <root_dir> <hostname> <port> <seq> <nb-times>'.format(sys.argv[0]))
    print('  e.g. {} <root_dir> <hostname> <port> ABCDEFEDCB 50'.format(sys.argv[0]))
    print()
    sys.exit(0)

conversion = {'A': 0., 'B': .2, 'C': .4, 'D': .6, 'E': .8, 'F': 1.}
root_dir  = sys.argv[1]
hostname  = sys.argv[2]
port      = int(sys.argv[3])
seq       = [conversion[c] for c in sys.argv[4]]
nb_times  = int(sys.argv[5])

with cx.variable.Realize(cx.variable.path_from(root_dir, 'in', 'xi')) as xi:
    for n in range(nb_times):
        for value in seq:
            xi += value


print('Sending "ping" to {}:{}'.format(hostname, port))
if cx.client.ping(hostname, port):
    print('Something went wrong.')
