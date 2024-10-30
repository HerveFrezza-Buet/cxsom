import sys
import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt


if len(sys.argv) != 3:
    print()
    print('Usage:')
    print(f'  {sys.argv[0]} <rootdir> <weight-timeline>')
    print()
    sys.exit(0)

root_dir = sys.argv[1]
wgt_timeline = sys.argv[2]
w_path   = cx.variable.path_from(root_dir, wgt_timeline , 'SOM/We-0') # W is <rootdir>/<wgt_timeline>/SOM/We-0.var

# This is a cxsom tool for viewing with matplotlib.
class MapViewer(cx.tkviewer.At):
    def __init__(self, master, title, figsize=(5, 4), dpi=100):
        super().__init__(master, title, figsize, dpi)
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        ax.set_aspect('equal')
        ax.set_xlim(-.2, 1.2)
        ax.set_ylim(-.2, 1.2)
        with cx.variable.Realize(w_path) as var:
            W = var[at]
            Wx, Wy = W[...,0], W[...,1]
            for X, Y in zip(Wx,   Wy  ): ax.plot(X, Y, 'k') # These two lines...
            for X, Y in zip(Wx.T, Wy.T): ax.plot(X, Y, 'k') # ... draw the grid.

# We open a GUI main window.
root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

# We add a slider synchronized with the instants stored in our W variable.
slider = cx.tkviewer.HistoryFromVariableSlider(root, 'from W', w_path)
slider.widget().pack(fill=tk.BOTH, side=tk.TOP)

# We add our viewer...
viewer = MapViewer(root, 'Map')
viewer.widget().pack(fill=tk.BOTH, side=tk.TOP)
viewer.set_history_slider(slider) # ... telling to draw the instant got from the slider.

# Then we start the GUI.
tk.mainloop()
