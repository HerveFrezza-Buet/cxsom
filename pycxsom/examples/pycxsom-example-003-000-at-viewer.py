import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt
import sys

# This viewer draws a sine curve. It inherits from cx.tkviewer. At,
# meaning that it is invoked when we need to draw something at a given
# timestep. This timestep is providing to the on_draw_at callback,
# that you have to override. Afterwards, when building is done, an
# history slider can be defined as the one providing our viewer with
# time step information.

class MyView(cx.tkviewer.At):
    def __init__(self, master, title, color, figsize=(5, 4), dpi=100):
        super().__init__(master, title, figsize, dpi)
        self.X = np.linspace(0, 5*np.pi, 200)
        self.color = color
        
    def on_draw_at(self, at):
        Y = np.sin(self.X + .1*at)
        self.fig.clear()
        ax = self.fig.gca()
        ax.set_ylim((-1.1, 1.1))
        ax.plot(self.X, Y, color=self.color)
    
root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

hs   = cx.tkviewer.HistorySlider(root, 'Main History', 0, 100, 50)
hbox = tk.Frame(root)
    
v1 = MyView(hbox, 'v1', '#ff0000')
v2 = MyView(hbox, 'v2', '#0000ff')
    
hs.widget().pack(side=tk.TOP, fill=tk.BOTH)
hbox.pack(side=tk.TOP, fill=tk.BOTH)
v1.widget().pack(side=tk.LEFT, fill=tk.BOTH)
v2.widget().pack(side=tk.LEFT, fill=tk.BOTH)

v1.set_history_slider(hs)
v2.set_history_slider(hs)

tk.mainloop()
