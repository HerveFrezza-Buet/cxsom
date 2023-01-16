import numpy as np
import matplotlib.pyplot as plt
import sys

if len(sys.argv) == 1 :
    print('usage : {} <tca-file1> <tca-file2> ...'.format(sys.argv[0]))

for tca in sys.argv[1:] :
    outname = tca.replace('.data', '.png')
    for num, line in enumerate(open(tca, 'r')) :
        if num == 1 :
            bmu = float(line)
        if num == 2 :
            argmax = float(line)
        if num == 3 :
            conv = np.array([float(s) for s in line.split()])
        if num == 4 :
            raw = np.array([float(s) for s in line.split()])
    fig = plt.figure(figsize=(10,5))
    plt.ylim((0, 1))
    plt.xlim((0, 1))
    color1 = '#1f77b4'
    color2 = '#ff7f0e'
    plt.plot(np.linspace(0, 1, len(raw),  endpoint=True), raw,  color=color1, alpha=.5)
    plt.plot(np.linspace(0, 1, len(conv), endpoint=True), conv, color=color1)
    plt.vlines(bmu, 0, 1, color=color2)
    plt.vlines(argmax, 0, 1, color=color2, alpha=.5, linewidth=5)
    plt.savefig(outname)
    print('fig "{}" generated.'.format(outname))
    plt.close(fig)
                            
    

