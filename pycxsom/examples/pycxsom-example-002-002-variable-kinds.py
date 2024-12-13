import os
import numpy as np
import pycxsom as cx

# We remove an eventual 'dummy_dir' directory.
os.system("rm -rf dummy_dir")
os.system("mkdir dummy_dir")

def varpath(name):
    return cx.variable.path_from('dummy_dir', 'dummy', name)

# Let us write different kinds of variables.

with cx.variable.Realize(varpath('A'), cx.typing.make('Scalar'), 1, 1) as x :
    x += 12.345
    
with cx.variable.Realize(varpath('B'), cx.typing.make('Pos1D'), 1, 1) as x :
    x += 12.345
    
with cx.variable.Realize(varpath('C'), cx.typing.make('Pos2D'), 1, 1) as x :
    value = np.zeros(x.datatype.shape(), dtype=float)
    value[0] = 12.345
    value[1] = 67.89
    x += value
    
with cx.variable.Realize(varpath('D'), cx.typing.make('Array=10'), 1, 1) as x :
    value = np.arange(10, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('E'), cx.typing.make('Map1D<Scalar>=10'), 1, 1) as x :
    value = np.arange(10, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('F'), cx.typing.make('Map1D<Pos1D>=10'), 1, 1) as x :
    value = np.arange(10, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('G'), cx.typing.make('Map1D<Pos2D>=5'), 1, 1) as x :
    value = np.arange(10, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('H'), cx.typing.make('Map1D<Array=3>=5'), 1, 1) as x :
    value = np.arange(15, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('I'), cx.typing.make('Map2D<Scalar>=5'), 1, 1) as x :
    value = np.arange(25, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('J'), cx.typing.make('Map2D<Pos1D>=5'), 1, 1) as x :
    value = np.arange(25, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('K'), cx.typing.make('Map2D<Pos2D>=5'), 1, 1) as x :
    value = np.arange(50, dtype=float).reshape(x.datatype.shape())
    x += value
    
with cx.variable.Realize(varpath('L'), cx.typing.make('Map2D<Array=3>=5'), 1, 1) as x :
    value = np.arange(75, dtype=float).reshape(x.datatype.shape())
    x += value

# Now, let us display the values we have stored.

for varname in ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L']:
    with cx.variable.Realize(varpath(varname)) as x :
        print('### {} : {}'.format(varname, x.datatype))
        print(x[0])
        print()



