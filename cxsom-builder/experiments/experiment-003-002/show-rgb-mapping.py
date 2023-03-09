import sys
import pycxsom as cx
import numpy as np
import matplotlib.pyplot as plt
import tkinter as tk


if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]
    

class RGBView(cx.tkviewer.At):
    def __init__(self, master, RGB=None, figsize=(10,10), dpi=100):
        super().__init__(master, 'RGB mapping', figsize, dpi)
        self.R = RGB[..., 0]
        self.G = RGB[..., 1]
        self.B = RGB[..., 2]
        self.COLOR = RGB
        
    def on_draw_at(self, at):
        self.fig.clear()
        ax = self.fig.add_subplot(projection='3d')
        ax.set_xlabel('R')
        ax.set_ylabel('G')
        ax.set_zlabel('B')
        ax.scatter(self.R, self.G, self.B, color=self.COLOR)
        with cx.variable.Realize(cx.variable.path_from(root_dir, 'saved', 'RGB/We-0')) as som:
            rgb = som[at]
            plt.plot(rgb[..., 0], rgb[..., 1], rgb[..., 2], lw=3, color='k')
        

RGB = np.fromiter((v for _,v in cx.variable.data_range_full(cx.variable.path_from(root_dir, 'img', 'rgb'))), dtype=(float, 3))

# RGB is too big, let us pick up a subset of it.
np.random.shuffle(RGB)
RGB = RGB[:2000]
    
root = tk.Tk()
root.protocol('WM_DELETE_WINDOW', lambda : sys.exit(0))

slider = cx.tkviewer.HistoryFromVariableSlider(root, 'Saved weights index', cx.variable.path_from(root_dir, 'saved', 'W/We-0'))
slider.widget().pack(fill=tk.BOTH, side=tk.TOP)

viewer = RGBView(root, RGB=RGB)
viewer.widget().pack(fill=tk.BOTH, side=tk.TOP)
viewer.set_history_slider(slider)

tk.mainloop()
