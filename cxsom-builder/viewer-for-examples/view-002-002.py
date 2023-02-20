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

class PrimaryMapView(cx.tkviewer.At):
    def __init__(self, master, map_name, figsize=(6, 3), dpi=100):
        super().__init__(master, "Primary map", figsize, dpi)
        self.map_name = map_name
        self.We_path  = cx.variable.path_from(root_dir, 'wgt', map_name+'/We-0')
        self.Wc_path  = cx.variable.path_from(root_dir, 'wgt', map_name+'/Wc-0')
        with cx.variable.Realize(self.We_path) as We:
            self.Xe = np.linspace(0, 1, We.datatype.shape()[0])
        with cx.variable.Realize(self.Wc_path) as Wc:
            self.Xc = np.linspace(0, 1, Wc.datatype.shape()[0])
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        with cx.variable.Realize(self.We_path) as We:
            Ye = We[at]
        with cx.variable.Realize(self.Wc_path) as Wc:
            Yc = Wc[at]
        ax.set_ylim(0, 1)
        ax.set_title('map "{}" weights at timestep #{}'.format(self.map_name, at))
        ax.plot(self.Xe, Ye, label='external')
        ax.plot(self.Xc, Yc, label='contextual')
        ax.legend()
        
class AssociativeMapView(cx.tkviewer.At):
    def __init__(self, master, map_name, figsize=(6, 3), dpi=100):
        super().__init__(master, "Associative map", figsize, dpi)
        self.map_name = map_name
        self.Wc0_path  = cx.variable.path_from(root_dir, 'wgt', map_name+'/Wc-0')
        self.Wc1_path  = cx.variable.path_from(root_dir, 'wgt', map_name+'/Wc-1')
        with cx.variable.Realize(self.Wc0_path) as Wc0:
            self.Xc0 = np.linspace(0, 1, Wc0.datatype.shape()[0])
        with cx.variable.Realize(self.Wc1_path) as Wc1:
            self.Xc1 = np.linspace(0, 1, Wc1.datatype.shape()[0])
        
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        with cx.variable.Realize(self.Wc0_path) as Wc0:
            Yc0 = Wc0[at]
        with cx.variable.Realize(self.Wc1_path) as Wc1:
            Yc1 = Wc1[at]
        ax.set_ylim(0, 1)
        ax.set_title('map "{}" weights at timestep #{}'.format(self.map_name, at))
        ax.plot(self.Xc0, Yc0, label='contextual 0')
        ax.plot(self.Xc1, Yc1, label='contextual 1')
        ax.legend()

root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

hbox = tk.Frame(root)

slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'time instants',
                                               cx.variable.path_from(root_dir, 'wgt', 'X/Wc-0'))

v1    = PrimaryMapView(hbox, 'X')
v2    = PrimaryMapView(hbox, 'Y')
v3    = AssociativeMapView(root, 'Assoc')
    
slider.widget().pack(side=tk.TOP,  fill=tk.BOTH)
hbox.pack           (side=tk.TOP,  fill=tk.BOTH)
v1.widget().pack    (side=tk.LEFT, fill=tk.BOTH)
v2.widget().pack    (side=tk.LEFT, fill=tk.BOTH)
v3.widget().pack    (side=tk.TOP,  fill=tk.BOTH)

v1.set_history_slider(slider)
v2.set_history_slider(slider)
v3.set_history_slider(slider)

tk.mainloop()

