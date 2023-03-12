import os
import sys
import time
import pycxsom as cx


if len(sys.argv) != 3:
    print()
    print('Usage:')
    print('  {} <root_dir> <image-side>'.format(sys.argv[0]))
    print()
    sys.exit(0)
    
root_dir   = sys.argv[1]
image_side = int(sys.argv[2])

with cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'W/Wc-0')) as var:
    _, last_frame_id = var.time_range()

print()
print(f'Generating frames until frame {last_frame_id}...')
print()
for frame_id in range(last_frame_id+1):
    os.system(f'make --quiet one-frame WEIGHTS_AT={frame_id} IMAGE_SIDE={image_side}')

            


