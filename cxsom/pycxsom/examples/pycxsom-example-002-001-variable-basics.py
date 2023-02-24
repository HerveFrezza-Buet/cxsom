import os
import sys
import pycxsom as cx
import numpy as np

# The variable is stored in a .var file. The path is built up from the
# root directory, wher all variables of a simulation are stored, a
# timeline (i.e. the first level of subdirectory), and a name, which
# can contain subdirectories as well.

if len(sys.argv) < 5:
    print()
    print('Usage:')
    print('  {} <root_dir> <timeline> <varname> COMMAND'.format(sys.argv[0]))
    print('where COMMAND is one of the following:')
    print('  realize <type> <cache_size> <file_size> // Realizes/creates the variable.')
    print('  range                                   // Shows the time range')
    print('  type                                    // Shows the variable type')
    print('  dump                                    // Shows the file content')
    print('  *@ <at>                                 // Reads a value')
    print('  +@ <at> <value>                         // Writes a value')
    print('  ++ <value>                              // Writes a at next timestep')
    print()
    print('Nota: The position <at> can be negative.')
    print()
    sys.exit(0)

root_dir = sys.argv[1]
timeline = sys.argv[2]
varname  = sys.argv[3]
command  = sys.argv[4]
args     = 5
nb_cmd_args = len(sys.argv) - 5

var_path = cx.variable.path_from(root_dir, timeline, varname)

print('File : {}'.format(var_path))


if command == 'realize':
    if nb_cmd_args != 3:
        print()
        print('Command usage: realize <type> <cache_size> <file_size>')
        sys.exit(1)
    var_type   = cx.typing.make(sys.argv[args + 0])
    cache_size = int(sys.argv[args + 1])
    file_size  = int(sys.argv[args + 2])
    # This creates/checks the file. After this call,
    # the object v is associated to the file.
    v = cx.variable.Realize(var_path, var_type, cache_size, file_size)
    print('done')
elif command == 'dump':
    print()
    for t, value in cx.variable.data_range_full(var_path) :
        print('{} @{}'.format(value, t))
elif command == 'type':
    with cx.variable.Realize(var_path) as v:
        print(v.datatype)
else:
    if command not in ['range', '*@', '+@', '++']:
        print()
        print('Command "{}" is not implemented'.format(command))
        sys.exit(1)
            
    # We open the variable and work with it. It should exist,
    # since we do not provide any information.
    with cx.variable.Realize(var_path) as v:
        try:
            if command == 'range':
                if nb_cmd_args != 0:
                    print()
                    print('Command usage: range')
                    sys.exit(0)
                r = v.time_range()
                if r:
                    print('Time range is [{}, {}]'.format(r[0], r[1]))
                else:
                    print('The file holds no data.')
            if command == '*@':
                if nb_cmd_args != 1:
                    print()
                    print('Command usage: *@ <at>')
                    sys.exit(0)
                at = int(sys.argv[args + 0])
                print('got {}'.format(v[at]))
            if command == '+@':
                if nb_cmd_args != 2:
                    print()
                    print('Command usage: *@ <at> <value>')
                    sys.exit(0)
                at    = int  (sys.argv[args + 0])
                value = float(sys.argv[args + 1])
                v[at] = np.full(v.datatype.shape(), value, dtype=np.float)
                print('done')
            if command == '++':
                if nb_cmd_args != 1:
                    print()
                    print('Command usage: ++ <value>')
                    sys.exit(0)
                value = float(sys.argv[args + 0])
                v += np.full(v.datatype.shape(), value, dtype=np.float)
                print('done')
        except cx.error.Busy:
            print('Nothing can be read, slot {} is busy'.format(at))
        except cx.error.Ready:
            print('Nothing can be written, slot {} is already ready'.format(at))
        except cx.error.Forgotten:
            print('Nothing can be accessed, slot {} is forgotten'.format(at))
        except cx.error.Any as err:
            print(err)
        except:
            print("Unexpected error:", sys.exc_info()[1])
