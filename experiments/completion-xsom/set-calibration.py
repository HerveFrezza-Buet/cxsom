import sys
import pycxsom as cx
import numpy as np

if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir  = sys.argv[1]

with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'pos-ref')) as pos_ref:
    pos_ref[0] = .5
    
with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'rgb-ref')) as rgb_ref:
    rgb_ref[0] = np.full(3, 1/3)
    
with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'pos-samples')) as pos_samples:
    grid_side = pos_samples.datatype.side
    pos_samples[0] = np.linspace(0, 1, grid_side)
    
with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'rgb-samples')) as rgb_samples:
    data = np.zeros((grid_side * grid_side, 3))
    idx = 0
    for g in np.linspace(0, 1, grid_side):
        for r in np.linspace(0, 1, grid_side):
            rg = r + g;
            if rg <= 1:
                b = 1 - rg
                data[idx, ...] = (r, g, b)
            idx += 1
    rgb_samples[0] = data




