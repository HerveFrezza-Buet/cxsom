import os
import sys
import time
import numpy as np
import pycxsom as cx
from pathlib import Path
import threading


PACK = 10


if len(sys.argv) != 6:
    print()
    print('Usage:')
    print('  {} <root_dir> <test-prefix> <frozen-prefix> <every> <next-frame>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
test_prefix = sys.argv[2]
frozen_prefix = sys.argv[3]
EVERY = int(sys.argv[4])
next_frame = int(sys.argv[5])


class System (threading.Thread):
   def __init__(self, command):
       threading.Thread.__init__(self)
       self.command = command
   def run(self):
       os.system(self.command)

# This function has reading/writing conflicts with the files, it is
# not robust (even if it is smarter).
def __wait_frozen_until(prefix, name, last):
    print(f'wait_frozen_until({prefix}, {name}, {last})')
    with cx.variable.Realize(cx.variable.path_from(root_dir, prefix, name)) as V:
        print('  sync...')
        V.sync_init() 
        print('  time_range')
        r = V.time_range()
        print(f'  r = {r}')
        if r == None:
            end = None
        else:
            _, end = r
        print(f'  end = {end}')
        while last != end:
            print('    wait_next...')
            V.wait_next(sleep_duration=1.0)
            r = V.time_range()
            print(f'    r = {r}')
            if r == None:
                end = None
            else:
                _, end = r
            print(f'  end = {end}')


ugly_file_size = None
def wait_frozen_until(prefix, name, last):
    global ugly_file_size
    last_size = -1
    path = cx.variable.path_from(root_dir, prefix, name)
    size = os.stat(str(path)).st_size
    if size == ugly_file_size:
        return
    if ugly_file_size == None:
        while size == 0 or size != last_size:
            time.sleep(2.0)
            last_size = size
            size = os.stat(str(path)).st_size
        ugly_file_size = size # we set the desired file size to the first stable one that we find... this is ugly.
        print(f'File sized estimated in the ugly way : {ugly_file_size}')
    else:
        while os.stat(str(path)).st_size < ugly_file_size:
            time.sleep(2.0)
        
    
    

with cx.variable.Realize(cx.variable.path_from(root_dir, 'wgt', 'X/We-0')) as W:
    data_size = W.time_range()[1]+1
    
with cx.variable.Realize(cx.variable.path_from(root_dir, test_prefix + '-in', 'X')) as W:
    test_last = W.time_range()[1]
    

simu = np.array([[frame_id, timestep] for frame_id, timestep in enumerate(range(0, data_size, EVERY))])
nb_splits = len(simu) // PACK
simu = np.array_split(simu, nb_splits)


os.system('make --quiet cxsom-launch-processor')
time.sleep(1)
for chunk in simu:
    first, last = chunk[0][0], chunk[-1][0]
    if chunk[-1][0] < next_frame:
        print(f'skipping frames [{first}... {last}]')
        continue
    os.system('make --quiet cxsom-clear-processor')
    os.system(f'make --quiet clear-frozen')
    time.sleep(.5)
    for _, timestep in chunk:
        print(f'sending frozen rules for timestep {timestep}')
        os.system(f'make --quiet send-frozen-rules TIMESTEP={timestep}')
    os.system(f'make --quiet cxsom-ping-processor')

    threads = []
    print(f'---- waiting the computation of {test_last+1} test steps.')
    print(f'     for frames [{first}... {last}].')
    for frame_id, timestep in chunk:
        prefix = '{}-{:08d}'.format(frozen_prefix, timestep)
        wait_frozen_until(prefix + '-out', 'X/BMU', test_last)
        wait_frozen_until(prefix + '-out', 'Y/BMU', test_last)
        wait_frozen_until(prefix + '-rlx', 'Cvg', test_last)
        thread = System(f'make --quiet view-frozen TIMESTEP={timestep} FRAME={frame_id}')
        print(f'>>>> starting generation of frame {frame_id} (step {timestep})')
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()
        del(thread)
        
    for frame_id, _ in chunk:
        print(f'<<<< frame {frame_id} generated.')
    
    

