import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    print()
    print('Usage:')
    print('  {} <root_dir> <timestep>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
timestep = int(sys.argv[2])
timeline_prefix = 'zfrz-{:08d}-'.format(timestep)
in_timeline  = timeline_prefix + 'in'
wgt_timeline = timeline_prefix + 'wgt'
rlx_timeline = timeline_prefix + 'rlx'
out_timeline = timeline_prefix + 'out'

def make_map_paths(map_name):
    return {'We'  : cx.variable.path_from(root_dir, wgt_timeline, map_name + '/We-0'),
            'Wc'  : cx.variable.path_from(root_dir, wgt_timeline, map_name + '/Wc-0'),
            'BMU' : cx.variable.path_from(root_dir, out_timeline, map_name + '/BMU')}

paths = {'MX'  : make_map_paths('X'),
         'MY'  : make_map_paths('Y'),
         'Cvg' : cx.variable.path_from(root_dir, rlx_timeline, 'Cvg'),
         'X'   : cx.variable.path_from(root_dir, in_timeline , 'X'),
         'Y'   : cx.variable.path_from(root_dir, in_timeline , 'Y'),
         'U'   : cx.variable.path_from(root_dir, in_timeline , 'U')}


def plot_convergence_histogram(ax):
    values = np.array([v for (_, v) in cx.variable.data_range_full(paths['Cvg'])])
    ax.set_title('Number of relaxation steps, {} samples used.'.format(len(values)))
    ax.hist(values, 100)
    

fig = plt.figure(figsize=(12,9))
ax = plt.gca()
plot_convergence_histogram(ax)
plt.show()




