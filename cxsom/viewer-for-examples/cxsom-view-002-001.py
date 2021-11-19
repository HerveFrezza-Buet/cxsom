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

# This is a viewer for several Map1D<Scalar> variables.

# Now, let us add a viewer. The idea is to inherit from a viewer
# class, and override your drawing function. Here, we want the viewer
# to handle the variables to be displayed.
class Viewer(cx.tkviewer.At):
    def __init__(self, master, title, root_dir, timeline, var_names, figsize=(5, 4), dpi=100):
        super().__init__(master, title, figsize, dpi)
        self.varpaths = [cx.variable.path_from(root_dir, timeline, name) for name in var_names]
        self.timeline = timeline

    # This is the inherited (and overrided) method for drawing
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        ax.set_title('Timeline {}'.format(self.timeline))
        ax.set_ylim(0, 1)
        nb_curves = 0
        for varpath in self.varpaths:
            _, timeline, name = cx.variable.names_from(varpath)
            with cx.variable.Realize(varpath) as v:
                try:
                    Y = v[at]
                    X = np.arange(len(Y))
                    ax.plot(X, Y, label=name)
                    nb_curves += 1
                except cx.error.Busy:
                    pass
                except cx.error.Forgotten:
                    pass
        if nb_curves > 0:
            ax.legend()

    
# Let us launch a graphical interface

# This is the main frame
root = tk.Tk()

# This is a time slider, associated to the first variable
varpath = cx.variable.path_from(root_dir, 'main', 'samplers/B')
slider = cx.tkviewer.HistoryFromVariableSlider(root, 'from {}'.format(varpath), varpath)
slider.widget().pack(fill=tk.BOTH, side=tk.TOP)

# We pack viewers horizontally.
hbox = tk.Frame(root)
hbox.pack(fill=tk.BOTH, side=tk.TOP)

# We add a first instance of our viewer in the GUI.
viewer = Viewer(hbox, 'The random samplers',
                root_dir, 'main',
                ['samplers/{}'.format(n) for n in 'BCD'])
viewer.widget().pack(fill=tk.BOTH, side=tk.LEFT)
viewer.set_history_slider(slider) # This viewer is controlled by our slider.

# We add a second instance of our viewer in the GUI.
viewer = Viewer(hbox, 'The averaged maps',
                root_dir, 'main',
                ['averagers/{}'.format(n) for n in 'ABCD'])
viewer.widget().pack(fill=tk.BOTH, side=tk.LEFT)
viewer.set_history_slider(slider) # This viewer is controlled by our slider.

# Then we start the GUI.
tk.mainloop()
