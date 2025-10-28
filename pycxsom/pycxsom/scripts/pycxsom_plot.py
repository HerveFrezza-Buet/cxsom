import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt
import sys

def collect_labels(varpath):
    curve_names = []
    with cx.variable.Realize(varpath) as data:
        if   isinstance(data.datatype, cx.typing.Scalar):
            curve_names = ['v']
        elif isinstance(data.datatype, cx.typing.Pos1D):
            curve_names = ['x']
        elif isinstance(data.datatype, cx.typing.Pos2D):
            curve_names = ['x', 'y']
        elif isinstance(data.datatype, cx.typing.Array):
            curve_names = ['a[{}]'.format(i) for i in range(data.datatype.dim)]
        else:
            print('This viewer does not handle data type {}.'.format(data.datatype))
    return curve_names
    
def collect_data(varpath, curve_names):
    zero = np.zeros(len(curve_names), dtype=np.float)
    X    = []
    Ys   = []
    mask = []
    for t, v in cx.variable.data_range_full(varpath):
        X.append(t)
        if v is not None :
            mask.append(False)
            if isinstance(v, float):
                Ys.append([v])
            else:
                Ys.append(v)
        else:
            mask.append(True)
            Ys.append(zero)
    return (np.array(X), np.array(Ys).T, np.array(mask))
    
def make_plot(title, X, Ys, mask, curve_names):
    ax  = plt.gca()
    ax.set_xlabel('time')
    ax.set_title(varpath)
    for label, curve in zip(curve_names, Ys):
        ax.plot(X, np.ma.masked_array(curve, mask=mask), label=label)
    ax.legend()

def main():
    if len(sys.argv) < 3:
        print('Usage : {} <file.var> [show | save] [save-filename]'.format(sys.argv[0]))
        sys.exit(1)
    
    varpath = sys.argv[1]
    command = sys.argv[2]
    
    if command not in ['show', 'save']:
        print('Command "{}" is not supported.'.format(command))
        sys.exit(1)
    
    curve_names = collect_labels(varpath)
    if len(curve_names) == 0:
        sys.exit(1)
    
    X, Ys, mask = collect_data(varpath, curve_names)
    
    fig = plt.figure(figsize=(10,3))
    make_plot(varpath, X, Ys, mask, curve_names)
    
    if command == 'show':
        plt.show()
        sys.exit(0)
    
    if command == 'save':
        if len(sys.argv) < 4:
            print('filename missing, aborting.')
            sys.exit(1)
        plt.savefig(sys.argv[3], bbox_inches='tight')
        print()
        print('file "{}" saved.'.format(sys.argv[3]))
        sys.exit(0)
    
        
            
            
    
    
        
    
    
