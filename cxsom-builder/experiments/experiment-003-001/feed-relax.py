import os
import sys
import pycxsom as cx
import numpy as np

if len(sys.argv) < 6:
    print()
    print('Usage:')
    print('  {} <root_dir> <hostname> <port> <expand-timeline-prefix> <theta> [Xbmu Ybmu]'.format(sys.argv[0]))
    print()
    sys.exit(0)


root_dir = sys.argv[1]
hostname = sys.argv[2]
port     = int(sys.argv[3])
timeline = sys.argv[4]
theta    = float(sys.argv[5])

x_var_path = cx.variable.path_from(root_dir, timeline + '-in', 'X')
y_var_path = cx.variable.path_from(root_dir, timeline + '-in', 'Y')


with cx.variable.Realize(x_var_path) as X:
    with cx.variable.Realize(y_var_path) as Y:
        t = theta*np.pi/180.0
        X[0] = .5 * (np.cos(t) + 1)
        Y[0] = .5 * (np.sin(t) + 1)


if len(sys.argv) == 8:
    bmux, bmuy = float(sys.argv[6]), float(sys.argv[7])
else:
    bmux, bmuy = np.random.random(2)
    
xbmu_var_path = cx.variable.path_from(root_dir, timeline + '-rlx', 'X/BMU')
ybmu_var_path = cx.variable.path_from(root_dir, timeline + '-rlx', 'Y/BMU')
with cx.variable.Realize(xbmu_var_path) as Xbmu:
    with cx.variable.Realize(ybmu_var_path) as Ybmu:
        Xbmu[0], Ybmu[0] = bmux, bmuy

print('Sending "ping" to {}:{}'.format(hostname, port))
if cx.client.ping(hostname, port):
    print('Something went wrong.')
