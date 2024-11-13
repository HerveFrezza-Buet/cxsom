import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt



if len(sys.argv) < 2:
    print()
    print('Usage:')
    print('  {} <root_dir>'.format(sys.argv[0]))
    print()
    sys.exit(1)

root_dir = sys.argv[1]
bmu_path    = cx.variable.path_from(root_dir, 'out', 'recSOM/BMU')
weight_path = cx.variable.path_from(root_dir, 'wgt', 'recSOM/We-0')

with cx.variable.Realize(weight_path) as v:
    map_size = len(v[-1])

times_bmus = np.fromiter(cx.variable.data_range_full(bmu_path), dtype=(float, 2))
weights    = np.fromiter((w for _, w in cx.variable.data_range_full(weight_path)), dtype=(float, map_size))

times = times_bmus[..., 0]
bmus  = times_bmus[..., 1]

fig = plt.figure()
ax = fig.gca()
ax.scatter(times, bmus, color='r', s=1, zorder=2)
ax.imshow(weights.T, vmin=0, vmax=1, cmap='gray', origin='lower', extent=(0, times[-1], 0, 1), zorder=1)
ax.set_aspect('auto')
plt.show()

