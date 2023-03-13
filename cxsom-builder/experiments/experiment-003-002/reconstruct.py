import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]

SRC = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img', 'rgb'))), dtype=(float, 3))
PRED = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'predict-out', 'rgb'))), dtype=(float, 3))

side = int(np.sqrt(len(SRC)))
SRC  = SRC.reshape (side, side, 3)
PRED = PRED.reshape(side, side, 3)


