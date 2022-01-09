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

def array_of_var(path):
    return np.array([v for (_, v) in cx.variable.data_range_full(path)])

def weigths_of_var(path):
    with cx.variable.Realize(path) as W:
        weight = W[0]
    bmu_one_pos = len(weight) - 1 # Beware, last index is bmu == 1.
    return lambda bmu : weight[(int)(bmu * bmu_one_pos + .5)]

paths = {'MX'  : make_map_paths('X'),
         'MY'  : make_map_paths('Y'),
         'Cvg' : cx.variable.path_from(root_dir, rlx_timeline, 'Cvg'),
         'X'   : cx.variable.path_from(root_dir, in_timeline , 'X'),
         'Y'   : cx.variable.path_from(root_dir, in_timeline , 'Y'),
         'U'   : cx.variable.path_from(root_dir, in_timeline , 'U')}

Xs    = array_of_var(paths['X'  ])
Ys    = array_of_var(paths['Y'  ])
Us    = array_of_var(paths['Y'  ])
CVGs  = array_of_var(paths['Cvg'])

We = {'X': weight_of_var(path['X']['We']),
      'Y': weight_of_var(path['Y']['We'])}
Wc = {'X': weight_of_var(path['X']['Wc']),
      'Y': weight_of_var(path['Y']['Wc'])}



def plot_convergence_histogram(ax):
    ax.set_title('Number of relaxation steps, {} samples used.'.format(len(CVGs)))
    ax.hist(CVGs, 100)

def plot_u_vs_bmu(ax, map_name):
    
    

fig = plt.figure(figsize=(12,9))
ax = plt.gca()
plot_convergence_histogram(ax)
plt.show()




