import os
import sys
import pycxsom as cx
import numpy as np



if len(sys.argv) != 6:
    print()
    print('Usage:')
    print(f'  {sys.argv[0]} <root_dir> <nb_interaction> <hostname> <port> <xrsw-port> ')
    print()
    sys.exit(0)

root_dir        = sys.argv[1]
nb_interactions = int(sys.argv[2])
hostname        = sys.argv[3]
port            = int(sys.argv[4])
xrswport        = int(sys.argv[5])

