import sys
import pycxsom as cx
import numpy as np
import tkinter as tk
import matplotlib as plt


if len(sys.argv) < 2:
    print()
    print('Usage:')
    print('  {} <varpath1> <varpath2> ... '.format(sys.argv[0]))
    print()
    sys.exit(0)

# Let us check that all the variables have a 1D shape.
varpaths = sys.argv[1:]
error = False
for var_path in varpaths :
    with cx.variable.Realize(var_path) as v:
        if isinstance(v.datatype, cx.typing.Map1D) and v.datatype.content.shape()[0] == 1 :
            pass
        else:
            print('{} contains type {} which is not a 1D map of scalars.'.format(var_path, v.datatype))
            error = True
if error:
    sys.exit(1)
    
    
# Let us launch a graphical interface

# This is the main frame
root = tk.Tk()

# This is a time slider, associated to the first variable varpaths[0]
slider = cx.tkviewer.HistoryFromVariableSlider(root,
                                               'from {}'.format(varpaths[0]),
                                               varpaths[0])
slider.widget().pack(fill=tk.BOTH, side=tk.TOP)

# Now, let us add a viewer. The idea is to inherit from a viewer
# class, and override your drawing function. Here, we want the viewer
# to handle the variables to be displayed.
class MyViewer(cx.tkviewer.At):
    def __init__(self, master, title, varpaths, figsize=(5, 4), dpi=100):
        super().__init__(master, title, figsize, dpi)
        self.varpaths = varpaths

    # This is the inherited (and overrided) method for drawing
    def on_draw_at(self, at):
        self.fig.clear()    # self.fig is the matplotlib figure
        ax = self.fig.gca()
        nb_curves = 0
        for varpath in self.varpaths:
            _, timeline, name = cx.variable.names_from(varpath)
            with cx.variable.Realize(varpath) as v:
                try:
                    Y = v[at]
                    X = np.arange(len(Y))
                    ax.plot(X, Y, label='({}){}'.format(timeline, name))
                    nb_curves += 1
                except cx.error.Busy:
                    pass
                except cx.error.Forgotten:
                    pass
        if nb_curves > 0:
            ax.legend()

# We add an instance of our viewer in the GUI.
viewer = MyViewer(root, 'My viewer', varpaths)
viewer.widget().pack(fill=tk.BOTH, side=tk.TOP)
viewer.set_history_slider(slider) # This viewer is controlled by our slider.

# Then we start the GUI.
tk.mainloop()
