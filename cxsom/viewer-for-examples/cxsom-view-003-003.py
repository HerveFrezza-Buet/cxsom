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
var_path = cx.variable.path_from(root_dir, 'wgt', 'W')


# let us build a viewer for our variable.

class MyViewer(cx.tkviewer.At):
    def __init__(self, master, figsize=(5, 5), dpi=100):
        super().__init__(master, "2D map 3D weights", figsize, dpi)
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        with cx.variable.Realize(var_path) as v:
            img = v[at]
            ax.imshow(img)


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
