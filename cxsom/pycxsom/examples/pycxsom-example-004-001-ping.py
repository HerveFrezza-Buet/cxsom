import sys
import pycxsom as cx


if len(sys.argv) < 3:
    print()
    print('Usage:')
    print('  {} <hostname> <port> ... '.format(sys.argv[0]))
    print()
    sys.exit(0)

hostname = sys.argv[1]
port     = int(sys.argv[2])

error = cx.client.ping(hostname, port)
if error :
    print('Error : {}'.format(error))
else:
    print('Done')

    
