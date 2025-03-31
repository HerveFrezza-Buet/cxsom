import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt


def get_weight_history(varpath):
    with cx.variable.Realize(varpath) as v:
        if not isinstance(v.datatype, cx.typing.Map1D):
            raise ValueError(f'Bad type for {varpath}: {v.datatype}')
        side = v.datatype.side
        if isinstance(v.datatype.content, cx.typing.Pos1D):
            depth = 1
        elif isinstance(v.datatype.content, cx.typing.Array) and v.datatype.content.dim == 3:
            depth = 3
        else:
            raise ValueError(f'Bad type for {varpath}: {v.datatype}')
    print(f'Reading history of {varpath}.')
    if depth == 1:
        return np.fromiter((value for _, value in cx.variable.data_range_full(varpath)), (float, side))
    return np.fromiter((value for _, value in cx.variable.data_range_full(varpath)), dtype=(float, (side, 3)))

def weight_history(ax, data, mapname, weight_kind, weight_rank, show_xticks, show_yticks):
    if weight_kind == 'c':
        ax.set_title(f'{mapname} (W{weight_kind}-{weight_rank})')
    else:
        ax.set_title(f'{mapname} (W{weight_kind})')
        
    if show_yticks:
        ax.set_ylabel('time')
    else:
        ax.set_yticks([])
        
    if not show_xticks:
        ax.set_xticks([])
        
    weights = data[mapname][weight_kind][weight_rank]
    if len(weights.shape) == 3:
        ax.imshow(weights, origin = 'lower')
    elif weight_kind == 'c':
        ax.imshow(weights, vmin = 0, vmax = 1, origin = 'lower', cmap = 'jet')
    else:
        ax.imshow(weights, vmin = 0, vmax = 1, origin = 'lower', cmap = 'gray')
        
        
