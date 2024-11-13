import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt



if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]



data = {'W':   {'e': [None], 'c': [None, None]},
        'H':   {'e': [None], 'c': [None, None]},
        'RGB': {'e': [None], 'c': [None, None]}}

data['W']  ['e'][0] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'W/We-0'))
data['H']  ['e'][0] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'H/We-0'))
data['RGB']['e'][0] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'RGB/We-0'))

data['W']  ['c'][0] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'W/Wc-0'))
data['H']  ['c'][0] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'H/Wc-0'))
data['RGB']['c'][0] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'RGB/Wc-0'))

data['W']  ['c'][1] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'W/Wc-1'))
data['H']  ['c'][1] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'H/Wc-1'))
data['RGB']['c'][1] = cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'RGB/Wc-1'))

with data['W']['e'][0] as var:
    _, last_frame_id = var.time_range()
    map_coef = len(var[0])-1
    POS = np.linspace(0, 1, len(var[0]))
    WGT_Y   = np.full_like(POS, -.1)

def idx(W):
    return (W*map_coef).astype(int)

s = 10
for frame_id in range(last_frame_id+1):
    
    with data['W']['e'][0] as var:
        We0 = var[frame_id]
    with data['W']['c'][0] as var:
        Wc0 = var[frame_id]
    with data['W']['c'][1] as var:
        Wc1 = var[frame_id]
        
    with data['H']['e'][0] as var:
        He0 = var[frame_id]
    with data['H']['c'][0] as var:
        Hc0 = var[frame_id]
    with data['H']['c'][1] as var:
        Hc1 = var[frame_id]
        
    with data['RGB']['e'][0] as var:
        Re0 = var[frame_id]
    with data['RGB']['c'][0] as var:
        Rc0 = var[frame_id]
    with data['RGB']['c'][1] as var:
        Rc1 = var[frame_id]
        
    fig = plt.figure(figsize=(16,5))

    plt.subplot(1,3,1)
    ax = fig.gca()
    ax.set_title('W map')
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_ylabel('weights to H map                    weights to RGB map')
    ax.scatter(POS, WGT_Y, c=We0,           cmap='gray', s=s)
    ax.scatter(POS, Wc0,   c=He0[idx(Wc0)], cmap='gray', s=s)
    ax.scatter(POS, Wc1+1, c=Re0[idx(Wc1)],              s=s)
    
    plt.subplot(1,3,2)
    ax = fig.gca()
    ax.set_title('H map')
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_ylabel('weights to W map                    weights to RGB map')
    ax.scatter(POS, WGT_Y, c=He0,           cmap='gray', s=s)
    ax.scatter(POS, Hc0,   c=We0[idx(Hc0)], cmap='gray', s=s)
    ax.scatter(POS, Hc1+1, c=Re0[idx(Hc1)],              s=s)
    
    plt.subplot(1,3,3)
    ax = fig.gca()
    ax.set_title('RGB map')
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_ylabel('weights to W map                  weights to H map')
    ax.scatter(POS, WGT_Y, c=Re0,                        s=s)
    ax.scatter(POS, Rc0,   c=We0[idx(Rc0)], cmap='gray', s=s)
    ax.scatter(POS, Rc1+1, c=He0[idx(Rc1)], cmap='gray', s=s)

    filename = 'frame-{:06d}.png'.format(frame_id)
    plt.savefig(filename, bbox_inches='tight')
    print(f'"{filename}" generated.')
    plt.close(fig)

