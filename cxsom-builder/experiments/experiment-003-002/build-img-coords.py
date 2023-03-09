import sys
import pycxsom as cx
import numpy as np

if len(sys.argv) < 4:
    print(f'Usage : {sys.argv[0]} <root-dir> <timeline> <img-side>')
    sys.exit(0)

root_dir = sys.argv[1]
timeline = sys.argv[2]
img_side = int(sys.argv[3])

W = np.linspace(0, 1, img_side)
H = np.linspace(0, 1, img_side)
Ws, Hs = np.meshgrid(W,H)

Ws = Ws.reshape(img_side**2)
Hs = Hs.reshape(img_side**2)

var_type = cx.typing.make('Pos1D')
with cx.variable.Realize(cx.variable.path_from(root_dir, timeline, 'w'), var_type, 2, img_side**2) as w:
    for value in Ws:
        w += value
with cx.variable.Realize(cx.variable.path_from(root_dir, timeline, 'h'), var_type, 2, img_side**2) as h:
    for value in Hs:
        h += value
    
