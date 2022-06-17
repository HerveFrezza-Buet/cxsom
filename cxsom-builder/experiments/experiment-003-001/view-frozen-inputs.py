import sys
import numpy as np
import matplotlib.pyplot as plt
import pycxsom as cx



if len(sys.argv) < 3:
    print()
    print('Usage:')
    print('  {} <root_dir> <test-prefix>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
test_prefix = sys.argv[2]

in_timeline  = test_prefix + '-in'

def array_of_var(path):
    return np.array([v for (_, v) in cx.variable.data_range_full(path)])

paths = {'X'   : cx.variable.path_from(root_dir, in_timeline , 'X'),
         'Y'   : cx.variable.path_from(root_dir, in_timeline , 'Y')}

INs    = {'X': array_of_var(paths['X']),
          'Y': array_of_var(paths['Y'])}

plt.figure()
plt.scatter(INs['X'], INs['Y'], s=10)
plt.show()
