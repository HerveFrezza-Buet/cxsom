import os
import sys
import time
import numpy as np
import pycxsom as cx
from pathlib import Path


PACK = 10


if len(sys.argv) != 5:
    print()
    print('Usage:')
    print('  {} <root_dir> <test-prefix> <frozen-prefix> <every>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
test_prefix = sys.argv[2]
frozen_prefix = sys.argv[3]
EVERY = int(sys.argv[4])


def wait_frozen_until(prefix, name, last):
    with cx.variable.Realize(cx.variable.path_from(root_dir, prefix, name)) as V:
        _, end = V.time_range()
        V.sync_init() 
        while last != end:
            V.wait_next(sleep_duration=1.0)
            _, end = V.time_range()
    

with cx.variable.Realize(cx.variable.path_from(root_dir, 'wgt', 'X/We-0')) as W:
    data_size = W.time_range()[1]+1
    
with cx.variable.Realize(cx.variable.path_from(root_dir, test_prefix + '-in', 'X')) as W:
    test_last = W.time_range()[1]
    

simu = np.array([[frame_id, timestep] for frame_id, timestep in enumerate(range(0, data_size, EVERY))])
nb_splits = len(simu) // PACK
simu = np.array_split(simu, nb_splits)


for chunk in simu:
    os.system('make cxsom-kill-processor')
    os.system(f'rm -rf {root_dir}/{frozen_prefix}*')
    os.system('make cxsom-launch-processor')
    time.sleep(1)
    for _, timestep in chunk:
        os.system(f'make send-frozen-rules TIMESTEP={timestep}')
    os.system(f'make cxsom-ping-processor')

    for frame_id, timestep in chunk:
        prefix = '{}-{:08d}'.format(frozen_prefix, timestep)
        wait_frozen_until(prefix + '-out', 'X/BMU', test_last)
        wait_frozen_until(prefix + '-out', 'Y/BMU', test_last)
        wait_frozen_until(prefix + '-rlx', 'Cvg', test_last)
        os.system(f'make view-frozen TIMESTEP={timestep} FRAME={frame_id}')
        print(f'#### frame {frame_id} generated.')
    
    

