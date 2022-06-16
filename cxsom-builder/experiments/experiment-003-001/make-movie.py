import os
import sys
import time
import pycxsom as cx

EVERY = 50


if len(sys.argv) < 2:
    print()
    print('Usage:')
    print('  {} <root_dir>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]


with cx.variable.Realize(cx.variable.path_from(root_dir, 'wgt', 'X/We-0')) as W:
    data_size = W.time_range()[1]+1

for frame_id, timestep in enumerate(range(0, data_size, EVERY)):
    os.system(f'make send-frozen-rules TIMESTEP={timestep}')
    os.system(f'make cxsom-ping-processor')
    time.sleep(3.0) # quite ugly...
    os.system(f'make view-frozen TIMESTEP={timestep} FRAME={frame_id}')
    print(f'frame {frame_id} generated.')
    
    

