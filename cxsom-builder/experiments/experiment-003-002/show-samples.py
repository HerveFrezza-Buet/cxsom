import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 4:
    print(f'Usage : {sys.argv[0]} <root-dir> <wh-timeline> <rbg-timeline> [frame_id]')
    sys.exit(0)

root_dir     = sys.argv[1]
wh_timeline  = sys.argv[2]
rgb_timeline = sys.argv[3]
frame_id = None
if len(sys.argv) == 5:
    frame_id = int(sys.argv[4])

with cx.variable.Realize(cx.variable.path_from(root_dir, wh_timeline, 'w')) as var:
    plot_range = var.time_range()

W = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, wh_timeline , 'w'  ))), float           )
H = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, wh_timeline , 'h'  ))), float           )
C = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, rgb_timeline, 'rgb'))), dtype=(float, 3))

plt.figure(figsize=(10,10))
if frame_id is None:
    plt.title(f'Inputs in {plot_range}')
plt.xlim(0,1)
plt.ylim(0,1)
plt.xticks([])
plt.yticks([])
plt.scatter(W, 1 - H, color=C)
if frame_id is None:
    plt.show()
else:
    filename = 'frame-{:06d}.png'.format(frame_id)
    plt.savefig(filename, bbox_inches='tight')
    print(f'image "{filename}" generated.')
