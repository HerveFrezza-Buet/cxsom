
import sys
import numpy as np
import matplotlib.pyplot as plt
import sample

if len(sys.argv) < 2:
    print()
    print('Usage:')
    print('  {sys.argv[0]} <shape>')
    print()
    sys.exit(0)
    
mode = sys.argv[1]

def make_dots(nb_intervals):
    Us = np.linspace(0, 1., nb_intervals+1, endpoint=True)
    XYs = np.array([sample.get(u, mode) for u in Us])
    return XYs[..., 0], XYs[..., 1]

plt.figure()
plt.gca().set_aspect('equal')
plt.title(mode)
margin=.05
plt.xlim(-margin, 1+margin)
plt.ylim(-margin, 1+margin)
Xs, Ys = make_dots(1000)
plt.scatter(Xs, Ys, s=10, zorder=1)
Xs, Ys = make_dots(10)
plt.scatter(Xs, Ys, s=20, zorder=2)
plt.show()
    
