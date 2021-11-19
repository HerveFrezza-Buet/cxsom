import sys
import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt

HISTORY_SIZE   = 50
ANGLE_ESPSILON = 5

if len(sys.argv) < 2:
    print()
    print('Usage:')
    print('  {} <root_dir>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]

def value_at(map1d, bmu, map_size):
    return map1d[(int)(bmu * (map_size - 1))]

def degree_of(bmu):
    return ANGLE_ESPSILON + 2 * bmu * (180 - ANGLE_ESPSILON)

def radian_of(bmu):
    return degree_of(bmu) * 0.017453292519943295

def color_of(wgt):
    return (wgt, wgt, wgt) # gray scale
    

class SeqView(cx.tkviewer.At):
    def __init__(self, master, figsize=(5, 5), dpi=100):
        super().__init__(master, "Learnt sequence", figsize, dpi)
        self.We_path  = cx.variable.path_from(root_dir, 'wgt', 'We')
        self.Wc_path  = cx.variable.path_from(root_dir, 'wgt', 'Wc')
        self.bmu_path = cx.variable.path_from(root_dir, 'out', 'bmu')
        with cx.variable.Realize(self.We_path) as We:
            self.map_size = We.datatype.shape()[0]
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        data = np.array([[value_at(We, bmu, self.map_size),
                          radian_of(value_at(Wc, bmu, self.map_size)),
                          radian_of(bmu)]
                         for (_, We), (_, Wc), (_, bmu) in zip(cx.variable.data_range_lasts_before(self.We_path,  HISTORY_SIZE, at),
                                                               cx.variable.data_range_lasts_before(self.Wc_path,  HISTORY_SIZE, at),
                                                               cx.variable.data_range_lasts_before(self.bmu_path, HISTORY_SIZE, at))],
                        dtype=float)
        ax.set_title('Weights at time step #{}'.format(at))
        ax.set_xlim(-1.3, 1.3)
        ax.set_ylim(-1.3, 1.3)
        ax.set_xticks([])
        ax.set_yticks([])
        ax.add_patch(plt.patches.Arc((0., 0.), 2., 2., theta1=degree_of(0), theta2=degree_of(1), color=(.8,.8,.9), zorder=0))
        X = np.cos(data[..., 2])
        Y = np.sin(data[..., 2])
        ax.scatter(X, Y, s=200, zorder=2, c=np.array([color_of(w) for w in data[..., 0]]), edgecolor='black')

        XX = np.cos(data[..., 1])
        YY = np.sin(data[..., 1])
        
        ax.add_collection(plt.collections.LineCollection(np.vstack((np.vstack((X,Y)), np.vstack((XX,YY)))).T.reshape(len(data),2,2),
                                                         colors=(.5, 0, 0), linewidths=1, zorder=1))
        

root = tk.Tk()

seq = SeqView(root)
slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'time instants',
                                               cx.variable.path_from(root_dir, 'wgt', 'We'))
slider.widget().pack(side=tk.TOP, fill=tk.BOTH)
seq.widget().pack(side=tk.LEFT, fill=tk.BOTH)
seq.set_history_slider(slider)

tk.mainloop()

