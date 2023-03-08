import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 3:
    print(f'Usage : {sys.argv[0]} <root-dir> <frame_id>')
    sys.exit(0)

root_dir = sys.argv[1]
frame_id = int(sys.argv[2])

W = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'prediction-input' , 'w'  ))), float           )
H = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'prediction-input' , 'h'  ))), float           )
C = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'prediction-output', 'rgb'))), dtype=(float, 3))

fig = plt.figure(figsize=(7,7))
plt.xlim(0,1)
plt.ylim(0,1)
plt.xticks([])
plt.yticks([])
plt.scatter(W, 1 - H, color=C)

if frame_id < 0:
    plt.show()
else:
    filename = 'frame-{:06d}.png'.format(frame_id)
    plt.figsave(filename, bbox_inches='tight')
    print(f'"{filename}" generated.')




