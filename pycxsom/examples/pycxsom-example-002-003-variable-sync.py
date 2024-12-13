import os
import sys
import time
import pycxsom as cx

if (len(sys.argv) == 1) or (sys.argv[1] not in ['generate', 'wait', 'listen']):
    print()
    print('Usage: {} <mode>'.format(sys.argv[0]))
    print()
    print('There are 3 modes for running the example.')
    print('   generate -> This cleans all, and writes periodically the variable.')
    print('               Start with this, and run other modes in another terminal.')
    print('   wait     -> This waits for next variable change, prints it, and exits.')
    print('   listen   -> This loops and notifies any changes.')
    print()
    exit()

mode    = sys.argv[1]
varpath = os.path.join('dummy_dir', 'timeline', 'ints.var')
                          
if mode == 'generate':
    os.system("rm -rf dummy_dir")
    os.system("mkdir -p dummy_dir/timeline")
    with cx.variable.Realize(varpath, cx.typing.make('Scalar'), 10, 10) as x :
        i = 0
        while True:
            time.sleep(2.0)
            x += i
            i += 1
                          
if mode == 'wait':
    with cx.variable.Realize(varpath) as x :
        x.sync_init() # we record the current time, from which we wait for a modification.
        print('waiting for new value...')
        x.wait_next(sleep_duration=.01) # We scan every 10ms to check for a value change.
        print('... got {} !'.format(x[-1]))

class Listener:
    def __init__(self):
        pass
    def on_new_value(self, v):
        print('v = {}'.format(v[-1]))
        
if mode == 'listen':
    l = Listener()
    with cx.variable.Realize(varpath) as x :
        x.sync_init() # we record the current time, from which we wait the first.
        print('Listening...')
        x.listen(l.on_new_value, sleep_duration=.01) # We scan every 10ms to check for a value change.
    
