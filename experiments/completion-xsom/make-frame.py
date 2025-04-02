import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt


if len(sys.argv) < 3:
    print(f'Usage : {sys.argv[0]} <root-dir> <frame-id>')
    sys.exit(0)

root_dir = sys.argv[1]
frame_id = int(sys.argv[2])


fig = plt.figure(figsize=(10,5))

plt.subplot(1,2,1)
ax = plt.gca()
ax.set_title('Checking')
W = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'check-out', 'W/We-0'  ))), float           )
H = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'check-out', 'H/We-0'  ))), float           )
C = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'check-out', 'RGB/We-0'))), dtype=(float, 3))
plt.xlim(0,1)
plt.ylim(0,1)
plt.xticks([])
plt.yticks([])
plt.scatter(W, 1 - H, color=C, s=10)

plt.subplot(1,2,2)
ax = plt.gca()
ax.set_title('Prediction')
W = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img'        , 'w'  ))), float           )
H = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img'        , 'h'  ))), float           )
C = np.fromiter((value for _, value in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'predict-out', 'rgb'))), dtype=(float, 3))
plt.xlim(0,1)
plt.ylim(0,1)
plt.xticks([])
plt.yticks([])
plt.scatter(W, 1 - H, color=C, s=5)


filename = 'frame-{:06d}.png'.format(frame_id)
plt.savefig(filename, bbox_inches='tight')
print(f'image "{filename}" generated.')
