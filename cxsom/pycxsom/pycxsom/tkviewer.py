
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import tkinter as tk
import numpy as np

from . import variable
from . import typing
from . import error
from . import sked

class HistorySlider:
    def __tickinterval(self, from_, to):
        r = to - from_
        if r < 10:
            return 1
        return (to - from_)//10
    
    def __init__(self, master, title, from_, to, init):
        self.at = tk.IntVar(master)
        self.at.set(init)
        self.frame1 = tk.Frame(master, borderwidth=5, relief=tk.FLAT)
        self.label_frame = tk.Frame(master, borderwidth=0, relief=tk.FLAT)
        self.title_lbl = tk.Label(self.label_frame, text=title)
        self.title_lbl.pack(side=tk.LEFT, fill=tk.BOTH)
        self.frame2 = tk.LabelFrame(self.frame1, borderwidth=2, relief=tk.GROOVE, labelwidget=self.label_frame)
        self.scale_frame = tk.Frame(self.frame2, borderwidth=5, relief=tk.FLAT)
        self.scale  = tk.Scale(self.scale_frame,
                               from_=from_, to=to, orient='horizontal',
                               resolution=1, tickinterval=self.__tickinterval(from_, to),
                               variable=self.at)
        self.frame2.pack(fill=tk.BOTH)
        self.scale_frame.pack(fill=tk.BOTH)
        self.scale.pack(fill=tk.BOTH)
        
    def widget(self):
        return self.frame1

    def redefine_bounds(self, from_, to):
        self.scale.destroy()
        self.scale = tk.Scale(self.scale_frame,
                              from_=from_, to=to, orient='horizontal',
                              resolution=1, tickinterval=self.__tickinterval(from_, to),
                              variable=self.at)
        self.scale.pack(fill=tk.BOTH)
        
        
class HistoryFromVariableSlider(HistorySlider):
    def __init__(self, master, title, var_path, skednet_lock=sked.nolock()):
        super().__init__(master, title, 0, 1, 0)
        self.refresh_button = tk.Button(self.label_frame, text='Refresh', command = self.on_refresh)
        self.refresh_button.pack(side=tk.LEFT, fill=tk.BOTH, padx=5)
        self.var_path = var_path
        self.xrsw = skednet_lock
        self.on_refresh()

    def on_refresh(self):
        with self.xrsw:
            with variable.Realize(self.var_path) as v:
                r = v.time_range()
                if r:
                    self.redefine_bounds(r[0], r[1])
    

class View:
    def __init__(self, master, title, figsize, dpi):
        self.at        = None
        self.title_str = title
        self.frame1    = tk.Frame(master, borderwidth=5, relief=tk.FLAT)
        self.label_frame =  tk.Frame(master, borderwidth=0, relief=tk.FLAT)
        self.title     = tk.StringVar()
        self.title.set(self.title_str)
        self.title_lbl = tk.Label(self.label_frame, textvariable=self.title)
        self.title_lbl.pack(side=tk.LEFT, fill=tk.BOTH)
        self.save_button = tk.Button(self.label_frame, text='Save', command = self.on_save)
        self.save_button.pack(side=tk.LEFT, fill=tk.BOTH, padx=5)
        self.refresh_button = tk.Button(self.label_frame, text='Refresh', command = self.draw)
        self.refresh_button.pack(side=tk.LEFT, fill=tk.BOTH, padx=(0, 5))
        self.frame2    = tk.LabelFrame(self.frame1, borderwidth=2, relief=tk.GROOVE, labelwidget=self.label_frame)
        self.frame3    = tk.Frame(self.frame2, borderwidth=5, relief=tk.FLAT)
        self.frame4    = tk.Frame(self.frame3, borderwidth=2, relief=tk.SUNKEN)
        self.frame2.pack(fill=tk.BOTH)
        self.frame3.pack(fill=tk.BOTH)
        self.frame4.pack(fill=tk.BOTH)
        container = self.frame4
        self.fig     = plt.figure(figsize=figsize, dpi=dpi)
        self.canevas = FigureCanvasTkAgg(self.fig, master=container)
        self.canevas.get_tk_widget().pack(fill=tk.BOTH)
        
    def on_save(self):
        filename = tk.filedialog.asksaveasfilename()
        if isinstance(filename, str):
            plt.figure(self.fig.number)
            plt.savefig(filename, bbox_inches='tight')
            
    def widget(self):
        return self.frame1

    def on_draw_at(self, at):
        pass
    
    def on_draw(self):
        pass
    
    def draw(self):
        if self.at is not None:
            at = self.at.get()
            self.title.set(self.make_title_label(at))
            self.on_draw_at(at)
        else:
            self.on_draw()
        self.canevas.draw()

class Refresh(View):
    def __init__(self, master, title, figsize, dpi):
        super().__init__(master, title, figsize, dpi)
        
class At(View):
    def __init__(self, master, title, figsize, dpi):
        super().__init__(master, title, figsize, dpi)

    def make_title_label(self, at):
        return '{} @{}'.format(self.title_str, at)
            
    def set_history_slider(self, history_slider):
        self.at = history_slider.at
        self.at.trace_add('write', self.on_value_changed)
        self.draw()

        
    def on_value_changed(self, a, b, c):
        self.draw()

    def set_range(self, from_, to, at):
        """
        at is an integer, or a control variable (e.g. other.at) of anoter scale.
        """
        self.__make_scale(from_, to, at)
        if type(at) is int:
            self.at.set(at)
        


