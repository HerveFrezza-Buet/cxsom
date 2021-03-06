import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

if len(sys.argv) < 4:
    print()
    print('Usage:')
    print('  {} <root_dir> <test-prefix> <frozen-prefix> <timestep> [frame-id]'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
test_prefix = sys.argv[2]
frozen_prefix = sys.argv[3]
timestep = int(sys.argv[4])
frame_id = None
if len(sys.argv) == 6:
    frame_id = int(sys.argv[5])
in_timeline  = test_prefix + '-in'
wgt_timeline = frozen_prefix + '-wgt'
rlx_timeline = frozen_prefix + '-rlx'
out_timeline = frozen_prefix + '-out'

s = 10 # scatter point size

def other(map_name):
    return {'X' : 'Y', 'Y': 'X'}[map_name]

def make_map_paths(map_name):
    return {'We'  : cx.variable.path_from(root_dir, wgt_timeline, map_name + '/We-0'),
            'Wc'  : cx.variable.path_from(root_dir, wgt_timeline, map_name + '/Wc-0'),
            'BMU' : cx.variable.path_from(root_dir, out_timeline, map_name + '/BMU')}

def array_of_var(path):
    return np.array([v for (_, v) in cx.variable.data_range_full(path)])

def weights_of_var(path):
    with cx.variable.Realize(path) as W:
        weight = W[0]
    return weight

def weight_fun_of_var(path):
    weight = weights_of_var(path)
    bmu_one_pos = len(weight) - 1 # Beware, last index is bmu == 1.
    return lambda bmu : weight[(int)(bmu * bmu_one_pos + .5)]

paths = {'MX'  : make_map_paths('X'),
         'MY'  : make_map_paths('Y'),
         'Cvg' : cx.variable.path_from(root_dir, rlx_timeline, 'Cvg'),
         'X'   : cx.variable.path_from(root_dir, in_timeline , 'X'),
         'Y'   : cx.variable.path_from(root_dir, in_timeline , 'Y'),
         'U'   : cx.variable.path_from(root_dir, in_timeline , 'U')}

INs    = {'X': array_of_var(paths['X']),
          'Y': array_of_var(paths['Y'])}
Us     = array_of_var(paths['U'  ])
CVGs   = array_of_var(paths['Cvg'])
BMUs   = {'X': array_of_var(paths['MX']['BMU']),
          'Y': array_of_var(paths['MY']['BMU'])}
We     = {'X': weights_of_var(paths['MX']['We']),
          'Y': weights_of_var(paths['MY']['We'])}
Wc     = {'X': weights_of_var(paths['MX']['Wc']),
          'Y': weights_of_var(paths['MY']['Wc'])}
We_fun = {'X': weight_fun_of_var(paths['MX']['We']),
          'Y': weight_fun_of_var(paths['MY']['We'])}
Wc_fun = {'X': weight_fun_of_var(paths['MX']['Wc']),
          'Y': weight_fun_of_var(paths['MY']['Wc'])}

MAP    = np.linspace(0, 1, len(We['X']))


def plot_inputs(ax):
    ax.set_title('Inputs')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.scatter(INs['X'], INs['Y'], s = s)
    
def plot_convergence_histogram(ax):
    ax.set_title('Number of relaxation steps, {} samples used.'.format(len(CVGs)))
    ax.hist(CVGs, 100)
    if frame_id != None:
        ax.set_xlim(0,105)
        ax.set_ylim(0,500)

def plot_bmu_vs_u(ax, map_name):
    ax.set_xlabel(map_name + '/BMU')
    ax.set_ylabel('U')
    ax.scatter(BMUs[map_name], Us, s = s)
    if frame_id != None:
        ax.set_xlim(-.05,1.05)
        ax.set_ylim(-.05,1.05)
    
def plot_input_fit(ax, map_name):
    ax.set_xlabel(map_name)
    ax.set_ylabel('We(bmu)')
    IN  = INs[map_name]
    W   = We_fun[map_name]
    BMU = BMUs[map_name]
    FIN = [W(bmu) for bmu in BMU]
    ax.scatter(IN, FIN, s = s)
    if frame_id != None:
        ax.set_xlim(-.1,1.1)
        ax.set_ylim(-.1,1.1)

def plot_map_match(ax, map_name):
    ax.set_xlabel('map ' + map_name)
    ax.plot(MAP, We[map_name], alpha=.5, label='We')
    ax.plot(MAP, Wc[map_name], alpha=.5, label='Wc')
    A = INs[map_name]
    B = INs[other(map_name)]
    BMU = BMUs[map_name]
    alpha = .2
    ax.scatter(BMU, A, alpha = alpha, s = s, label = map_name)
    ax.scatter(BMU, B, alpha = alpha, s = s, label = other(map_name))
    ax.scatter(BMU, np.zeros_like(BMU) - .1, alpha = alpha, color='k', s = s, label = 'BMU')
    if frame_id != None:
        ax.set_xlim(-.05,1.05)
        ax.set_ylim(-.2,1.1)
    if frame_id == None:
        ax.legend()

def plot_in_space(ax, map_name):
    ax.set_xlabel('map ' + map_name + ' in input space')
    I = np.argsort(BMUs[map_name])
    Bx = BMUs['X'][I]
    By = BMUs['Y'][I]
    Wx = We_fun['X']
    Wy = We_fun['Y']
    
    ax.scatter(INs['X'], INs['Y'], s = s, alpha=.1, zorder=0)
    X = np.array([Wx(bmu) for bmu in Bx])
    Y = np.array([Wy(bmu) for bmu in By])
    ax.plot(X, Y, c='k', alpha=.1, zorder=1)
    ax.scatter(X, Y, s = s, c='k', alpha=.1, zorder=2)
    if frame_id != None:
        ax.set_xlim(-.1,1.1)
        ax.set_ylim(-.1,1.1)
    
    


fig = plt.figure(figsize=(15,9), constrained_layout=True)
gs = GridSpec(3, 5, figure=fig)

ax = fig.add_subplot(gs[0, 0])
plot_inputs(ax)
ax = fig.add_subplot(gs[0, 1:])
plot_convergence_histogram(ax)

ax = fig.add_subplot(gs[1, 0:2])
plot_map_match(ax, 'X')
ax = fig.add_subplot(gs[1, 2])
plot_bmu_vs_u(ax, 'X')
ax = fig.add_subplot(gs[1, 3])
plot_input_fit(ax, 'X')
ax = fig.add_subplot(gs[1, 4])
plot_in_space(ax, 'X')

ax = fig.add_subplot(gs[2, 0:2])
plot_map_match(ax, 'Y')
ax = fig.add_subplot(gs[2, 2])
plot_bmu_vs_u(ax, 'Y')
ax = fig.add_subplot(gs[2, 3])
plot_input_fit(ax, 'Y')
ax = fig.add_subplot(gs[2, 4])
plot_in_space(ax, 'Y')

if frame_id != None:
    plt.savefig('frame-{:06d}.png'.format(frame_id), bbox_inches='tight')
else:
    plt.savefig('snap-{:08d}.pdf'.format(timestep), bbox_inches='tight')
    plt.show()




