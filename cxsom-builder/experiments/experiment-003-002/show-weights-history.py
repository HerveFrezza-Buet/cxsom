import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

import display


if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]

data = {'W':   {'e': [None], 'c': [None, None]},
        'H':   {'e': [None], 'c': [None, None]},
        'RGB': {'e': [None], 'c': [None, None]}}
grid_pos = {'W':   {'e': [(0, 0)], 'c': [(1, 0), (2, 0)]},
            'H':   {'e': [(0, 1)], 'c': [(1, 1), (2, 1)]},
            'RGB': {'e': [(0, 2)], 'c': [(1, 2), (2, 2)]}}

data['W']  ['e'][0] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'W/We-0'))
data['H']  ['e'][0] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'H/We-0'))
data['RGB']['e'][0] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'RGB/We-0'))

data['W']  ['c'][0] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'W/Wc-0'))
data['H']  ['c'][0] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'H/Wc-0'))
data['RGB']['c'][0] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'RGB/Wc-0'))

data['W']  ['c'][1] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'W/Wc-1'))
data['H']  ['c'][1] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'H/Wc-1'))
data['RGB']['c'][1] = display.get_weight_history(cx.variable.path_from(root_dir, 'saved', 'RGB/Wc-1'))

fig = plt.figure(figsize=(12,8))
gs = fig.add_gridspec(3, 3, wspace=0.05, hspace=0.05)

for mapname, mapweights in grid_pos.items():
    for weight_kind, positions in mapweights.items():
        for idx, (h, w) in enumerate(positions):
            display.weight_history(fig.add_subplot(gs[h, w]), data, mapname, weight_kind, idx, h==2, w==0)
plt.show()

