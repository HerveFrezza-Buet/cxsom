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


class DataView(cx.tkviewer.At):
    def __init__(self, master, figsize=(8, 5), dpi=100):
        super().__init__(master, "mid-bound (foo)", figsize, dpi)
        self.X_path = [cx.variable.path_from(root_dir, 'in', f'X-{i}') for i in range(10)]
        self.Y_path =  cx.variable.path_from(root_dir, 'foo', 'Y')
        with cx.variable.Realize(self.Y_path) as Y:
            self.abscissas = np.arange(Y.datatype.shape()[0])
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        ax.set_ylim(0, 1)
        for i in range(10):
            with cx.variable.Realize(self.X_path[i]) as X:
                ax.scatter(self.abscissas, X[at], s=10, alpha=.5, c='b', zorder=0)
        with cx.variable.Realize(self.Y_path) as Y:
            ax.scatter(self.abscissas, Y[at], s=10, alpha=1, c='r', zorder=1)
        
        
root = tk.Tk()
slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'time instants',
                                               cx.variable.path_from(root_dir, 'foo', 'Y'))
data = DataView(root)

slider.widget().pack(side=tk.TOP,  fill=tk.BOTH)
data.widget().pack(side=tk.TOP,  fill=tk.BOTH)

data.set_history_slider(slider)

tk.mainloop()
