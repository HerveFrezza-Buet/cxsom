import sys
import pycxsom as cx
import math
import matplotlib.pyplot as plt



if len(sys.argv) != 6:
    print()
    print('Usage:')
    print(f'  {sys.argv[0]} <root_dir> <nb_interaction> <hostname> <port> <xrsw-port> ')
    print()
    sys.exit(0)

root_dir        = sys.argv[1]
nb_interactions = int(sys.argv[2])
hostname        = sys.argv[3]
port            = int(sys.argv[4])
xrsw_port       = int(sys.argv[5])

def ping():
    if cx.client.ping(hostname, port):
        print('Pinging failed, something went wrong.')
        sys.exit(1)

use_locks = True # change it to False, you will get wrong behavior.

if use_locks:
    read_lock  = cx.sked.read( hostname, xrsw_port)
    write_lock = cx.sked.write(hostname, xrsw_port)
else:
    read_lock  = cx.sked.nolock()
    write_lock = cx.sked.nolock()

# We lock for reading (only initialization here).
with read_lock:
    min_sensor = cx.variable.Realize(cx.variable.path_from(root_dir, 'out', 'min'))
    max_sensor = cx.variable.Realize(cx.variable.path_from(root_dir, 'out', 'max'))
    cmd        = cx.variable.Realize(cx.variable.path_from(root_dir, 'in' , 'X'  ))
    # Then, we record current timesteps, in order to detect actual
    # transitions by try_wait_next.
    with min_sensor:
        min_sensor.sync_init()
    with max_sensor:
        max_sensor.sync_init()
            

CMDs = []
MINs = []
MAXs = []
t = 0
for transition in range(nb_interactions):

    with cx.sked.write(hostname, xrsw_port):
        with cmd:
            value  = .5*(1+math.sin(.3*t))
            t     += 1
            cmd   += value
            CMDs.append(value)
            
    # We wake up the processor (if needed)
    ping()

    # We wait for next data... This reading has to be protected.
    min_waiting = True
    max_waiting = True
    while min_waiting or max_waiting:
        with read_lock: 
            if min_waiting:
                with min_sensor:
                    min_waiting = min_sensor.try_wait_next()
                    if not min_waiting:
                        MINs.append(min_sensor[-1])
            if max_waiting:
                with max_sensor:
                    max_waiting = max_sensor.try_wait_next()
                    if not max_waiting:
                        MAXs.append(max_sensor[-1])

plt.figure(figsize=(.2*len(CMDs), 2))
plt.ylim(0, 1)
plt.plot(MINs, label='mins')
plt.plot(MAXs, label='maxs')
plt.plot(CMDs, label='cmds')
plt.legend()
plt.savefig('episode.pdf', bbox_inches='tight')
        
    
    


        
    
    
