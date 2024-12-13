import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt
import sys

# This viewer draws a sine curve, whoe phase changes each time the
# user pressed the "Refresh" button. We only have to override the
# on_draw method in order to redraw when refresh is pressed.

class MyView(cx.tkviewer.Refresh):
    def __init__(self, master, title, figsize=(5, 4), dpi=100):
        super().__init__(master, title, figsize, dpi)
        self.X = np.linspace(0, 5*np.pi, 200)
        self.phase = 0
        self.draw() # This forces a first drawing
        
    def on_draw(self):
        Y = np.sin(self.X + self.phase)
        self.fig.clear()
        ax = self.fig.gca()
        ax.set_ylim((-1.1, 1.1))
        ax.plot(self.X, Y)
        self.phase += .1
        if self.phase > 2*np.pi:
            self.phase -= 2*np.pi
    
root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

v = MyView(root, 'Translating sine')
v.widget().pack(side=tk.LEFT, fill=tk.BOTH)

tk.mainloop()
