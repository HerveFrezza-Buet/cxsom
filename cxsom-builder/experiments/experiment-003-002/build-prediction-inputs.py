import sys
import pycxsom as cx
import numpy as np

if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]

nb_steps = 100
W = np.linspace(0, 1, nb_steps)
H = np.linspace(0, 1, nb_steps)
Ws, Hs = np.meshgrid(W,H)

Ws = Ws.reshape(nb_steps**2)
Hs = Hs.reshape(nb_steps**2)

var_type = cx.typing.make('Pos1D')
with cx.variable.Realize(cx.variable.path_from(root_dir, 'prediction-input', 'w'), var_type, 2, nb_steps**2) as w:
    for value in Ws:
        w += value
with cx.variable.Realize(cx.variable.path_from(root_dir, 'prediction-input', 'h'), var_type, 2, nb_steps**2) as h:
    for value in Hs:
        h += value
    
