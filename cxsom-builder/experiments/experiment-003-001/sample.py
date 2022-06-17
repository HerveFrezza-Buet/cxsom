import numpy as np

mode = 'banana'

def get(U):
    if mode == 'circle':
        t = 2 * np.pi * U
        return (.5 * (1 + np.cos(t)),
                .5 * (1 + np.sin(t)))
    if mode == 'banana':
        t = 2 * np.pi * U
        return ((1 +  np.cos(2 * t) * (1 - np.exp(-.25 * (U - .5)**2))) * .5,
                .5 * (1 + np.sin(t)))
                 
