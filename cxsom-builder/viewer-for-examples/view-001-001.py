import sys
import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt


if len(sys.argv) < 2:
    print()
    print('Usage:')
    print('  {} <root_dir>'.format(sys.argv[0]))
    print()
    sys.exit(0)

root_dir = sys.argv[1]
var_path = cx.variable.path_from(root_dir, 'wgt', 'SOM/We-0')


# let us build a viewer for our variable.

class MyViewer(cx.tkviewer.At):
    def __init__(self, master, figsize=(10, 4), dpi=100):
        super().__init__(master, "1D weights", figsize, dpi)
        with cx.variable.Realize(var_path) as v:
            self.X = np.arange(v.datatype.shape()[0])
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        with cx.variable.Realize(var_path) as v:
            Wgts = v[at]
        ax.set_ylim((0,1))
        ax.plot(self.X, Wgts)


root = tk.Tk()

# This is a time slider, associated to the first variable varpaths[0]
slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'Spanning {}'.format(var_path),
                                               var_path)
slider.widget().pack(fill=tk.BOTH, side=tk.TOP)
    
viewer = MyViewer(root)
viewer.widget().pack(fill=tk.BOTH, side=tk.TOP)
viewer.set_history_slider(slider) # This viewer is controlled by our slider.

# Then we start the GUI.
tk.mainloop()
