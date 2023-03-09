import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]

with cx.variable.Realize(cx.variable.path_from(root_dir, 'img', 'w')) as var:
    plot_range = var.time_range()

W = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img', 'w'  ))), float           )
H = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img', 'h'  ))), float           )
C = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img', 'rgb'))), dtype=(float, 3))

plt.figure(figsize=(10,10))
plt.title(f'Inputs in {plot_range}')
plt.xlim(0,1)
plt.ylim(0,1)
plt.xticks([])
plt.yticks([])
plt.scatter(W, 1 - H, color=C)
plt.show()
