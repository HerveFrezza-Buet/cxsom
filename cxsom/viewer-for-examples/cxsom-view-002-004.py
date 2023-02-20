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

class MyView(cx.tkviewer.Refresh):
    def __init__(self, master, title, figsize=(10, 4), dpi=100):
        super().__init__(master, title, figsize, dpi)
        self.var_path = cx.variable.path_from(root_dir, 'relax', 'Cvg')
        self.draw() # This forces a first drawing

    def on_draw(self):
        X = []
        Y = []
        for t, value in cx.variable.data_range_full(self.var_path) :
            X.append(t)
            Y.append(value)
        self.fig.clear()
        ax = self.fig.gca()
        ax.set_ylim((5, 15))
        ax.plot(np.array(X), np.array(Y))    
        
root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

v = MyView(root, 'Convergence')
v.widget().pack(side=tk.LEFT, fill=tk.BOTH)
tk.mainloop()

    
