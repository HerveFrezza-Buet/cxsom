import sys
import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt


if len(sys.argv) != 3:
    print()
    print('Usage:')
    print('  {} <root_dir> <timestep>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
timestep = int(sys.argv[2])
timeline_prefix = 'zrlx-{:08d}-'.format(timestep)
wgt_timeline = timeline_prefix + 'wgt'
rlx_timeline = timeline_prefix + 'rlx'

class MapView(cx.tkviewer.At):
    def __init__(self, master, me, other, at_weight, figsize=(8, 5), dpi=100):
        super().__init__(master, "Map {} at {}".format(me, at_weight), figsize, dpi)
        self.me    = me
        self.other = other
        self.at_weight = at_weight;
        self.We_path  = cx.variable.path_from(root_dir, wgt_timeline, me    + '/We-0')
        self.Wc_path  = cx.variable.path_from(root_dir, wgt_timeline, me    + '/Wc-0')
        self.Ae_path  = cx.variable.path_from(root_dir, rlx_timeline, me    + '/Ae-0')
        self.Ac_path  = cx.variable.path_from(root_dir, rlx_timeline, me    + '/Ac-0')
        self.A_path   = cx.variable.path_from(root_dir, rlx_timeline, me    + '/A'   )
        self.BMU_path = cx.variable.path_from(root_dir, rlx_timeline, me    + '/BMU' )
        self.Ctx_path = cx.variable.path_from(root_dir, rlx_timeline, other + '/BMU' )
        with cx.variable.Realize(self.We_path) as We:
            self.X  = np.linspace(0, 1, We.datatype.shape()[0])
            self.We = We[-1]
        with cx.variable.Realize(self.Wc_path) as Wc:
            self.Wc = Wc[-1]
        
    def on_draw_at(self, at):
        with cx.variable.Realize(self.Ae_path) as A:
            Ae  = A[at]
        with cx.variable.Realize(self.Ac_path) as A:
            Ac  = A[at]
        with cx.variable.Realize(self.A_path) as A:
            A   = A[at]
        with cx.variable.Realize(self.BMU_path) as v:
            BMU = v[at]
        with cx.variable.Realize(self.Ctx_path) as v:
            ctx = v[at]
        
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        ax.set_xlim(0, 1)
        ax.set_ylim(0, 1)
        ax.set_title('Relaxation step #{} in map {} at {}'.format(at, self.me, self.at_weight))
        ax.plot(self.X, self.We, alpha=.5, ls='--', label='We')
        ax.plot(self.X, self.Wc, alpha=.5, ls='--', label='Wc')
        ax.plot(self.X,      Ae,           label='Ae')
        ax.plot(self.X,      Ac,           label='Ac')
        ax.plot(self.X,      A ,           label='A' )
        ax.axvline(BMU, color='k')
        ax.axhline(ctx, color='k', ls='--')
        ax.legend()

root = tk.Tk()

slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'Relaxation step',
                                               cx.variable.path_from(root_dir, rlx_timeline, 'X/BMU'))

hbox = tk.Frame(root)
v1   = MapView(hbox, 'X', 'Y', timestep)
v2   = MapView(hbox, 'Y', 'X', timestep)

slider.widget().pack(side=tk.TOP,  fill=tk.BOTH)
hbox.pack           (side=tk.TOP,  fill=tk.BOTH)
v1.widget().pack    (side=tk.LEFT, fill=tk.BOTH)
v2.widget().pack    (side=tk.LEFT, fill=tk.BOTH)

v1.set_history_slider(slider)
v2.set_history_slider(slider)

tk.mainloop()
