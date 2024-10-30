# file feed.py

import os
import sys
import pycxsom as cx
import numpy as np

if len(sys.argv) < 3:
    print()
    print('Usage:')
    print('  {} [crown|square|triangle] <root_dir> [<hostname> <port>]'.format(sys.argv[0]))
    print()
    sys.exit(0)

distrib   = sys.argv[1]
root_dir  = sys.argv[2]

nb_to_send = 5000 # We send a bunch of samples of that size.

# We access to the data file <rootdir>/in/xi.var
with cx.variable.Realize(cx.variable.path_from(root_dir, 'in', 'xi')) as xi:
    while nb_to_send > 0: # We may produce less than nb_to_send samples at each iteration.
        Xs = np.random.rand(nb_to_send)
        Ys = np.random.rand(nb_to_send)
        if distrib == 'crown':
            rmin, rmax = .35, .5
            R2s = (Xs - .5)**2 + (Ys - .5)**2
            keep = np.logical_and(rmin**2 < R2s, R2s < rmax**2)
            Xs, Ys = Xs[keep], Ys[keep]
        elif distrib == 'triangle':
            keep = Xs > Ys
            Xs, Ys = Xs[keep], Ys[keep]
        elif distrib == 'square':
            pass
        else:
            print()
            print()
            print('Distribution {} is invalid'.format(distrib))
            print()
            sys.exit(0)
        
        if len(Xs) > nb_to_send :
            sent = nb_to_send
            Xs, Ys = Xs[:nb_to_send], Ys[:nb_to_send]
        else:
            sent = len(Xs)

        # We format the data as [[x, y], [x, y], ....]
        XYs = np.stack((Xs, Ys)).T
        # Each line is a [x, y] that we add to the xi file.
        for xy in XYs: xi += xy
        
        nb_to_send -= sent

# We tell the server to wake up since we have added new data that should
# unlock computation.
if len(sys.argv) == 5:
    hostname  = sys.argv[3]
    port      = int(sys.argv[4])
    print('Sending "ping" to {}:{}'.format(hostname, port))
    if cx.client.ping(hostname, port):
        print('Something went wrong.')
