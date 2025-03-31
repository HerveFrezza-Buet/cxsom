import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir  = sys.argv[1]


fig = plt.figure(figsize=(10, 5))

plt.subplot(2,1,1)
with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'pos-match')) as pos_match:
    Y = pos_match[0]
    grid_side = len(Y)
X = np.linspace(0, 1, grid_side)
plt.title('Matching curve for input positions')
plt.xticks([])
plt.plot(X, Y)

plt.subplot(2,1,2)
with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'ctx-match')) as ctx_match:
    Y = ctx_match[0]
plt.title('Matching curve for contextual positions')
plt.plot(X, Y)


fig = plt.figure(figsize=(10, 10))
ax = fig.add_subplot(projection='3d')
with cx.variable.Realize(cx.variable.path_from(root_dir, 'calibration', 'rgb-match')) as rgb_match:
    Z = rgb_match[0].reshape(grid_side, grid_side)
X, Y = np.meshgrid(X, X)
ax.set_title('Matching curve for (RGB), expressed in the CIE xy plan.')
ax.set_xlabel('CIE x')
ax.set_ylabel('CIE y')
ax.set_zlabel('Matching')
ax.plot_surface(X, Y, Z, edgecolor='red', color='#eeeeee', lw=.5, rstride=2, cstride=2, alpha=.5)


plt.show()


