import matplotlib.pyplot as plt
import numpy as np

import pycxsom as cx

side = 100

r = np.linspace(0, 1, side, endpoint = True)
X = np.repeat(r.reshape(1,side), side, axis=0)
Y = X.T

# This is the data you could get from Map2d<Scalar>=100
# or a Map2d<Pos1D>=100
W_2d_Pos1d = .5*(X+Y)

# This is the data you could get from Map2d<Pos2D>=100
W_2d_Pos2d = np.dstack((Y,X))

# Let us plot the weigh grids.

fig = plt.figure(figsize=(5,5))
plt.title('Grid of scalars')
cx.plot.grid_of_1D(fig.gca(), W_2d_Pos1d)

fig = plt.figure(figsize=(5,5))
plt.title('Grid of pairs in [0,1]')
cx.plot.grid_of_2D(fig.gca(), W_2d_Pos2d)

plt.show()
