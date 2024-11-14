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

class MapView(cx.tkviewer.At):
    def __init__(self, master, map_name, figsize=(8, 5), dpi=100):
        super().__init__(master, "Bimodal map", figsize, dpi)
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
        
class HistoView(cx.tkviewer.Refresh):
    def __init__(self, master, figsize=(8, 4), dpi=100):
        super().__init__(master, 'Relaxation step durations', figsize, dpi)
        self.draw() # This forces a first drawing
        
    def on_draw(self):
        self.fig.clear()
        ax = self.fig.gca()
        values = np.array([v for (_, v) in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'rlx', 'Cvg'))])
        ax.set_title('Number of relaxation steps, {} samples used.'.format(len(values)))
        ax.hist(values, 100)

root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'time instants',
                                               cx.variable.path_from(root_dir, 'wgt', 'X/We-0'))

hbox1 = tk.Frame(root)
v1    = MapView(hbox1, 'X')
v2    = MapView(hbox1, 'Y')
                                                                  
hbox2 = tk.Frame(root)
h    = HistoView(hbox2)
    
slider.widget().pack(side=tk.TOP,  fill=tk.BOTH)
hbox1.pack          (side=tk.TOP,  fill=tk.BOTH)
v1.widget().pack    (side=tk.LEFT, fill=tk.BOTH)
v2.widget().pack    (side=tk.LEFT, fill=tk.BOTH)
hbox2.pack          (side=tk.TOP,  fill=tk.BOTH)
h.widget().pack     (side=tk.TOP, fill=tk.BOTH)

v1.set_history_slider(slider)
v2.set_history_slider(slider)

tk.mainloop()

