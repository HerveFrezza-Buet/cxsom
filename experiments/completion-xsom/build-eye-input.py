import sys
import pycxsom as cx
import numpy as np

"""
This opens eye.ppm and "copies" it into a cxsom variable (img/src), at time 0 pnly, where RGB values are rescaled from [0, 255] to [0, 1].
"""

if len(sys.argv) < 2:
    print(f'Usage : {sys.argv[0]} <root-dir>')
    sys.exit(0)

root_dir = sys.argv[1]
with open('eye.ppm', 'rb') as img_file:
    line = img_file.readline()
    if line[:2].decode('ascii') != 'P6':
        raise ValueError('eye.ppm is not a color by=inary PPM file (P6)')
    line = img_file.readline()
    while line.decode('ascii')[0] == '#':
        line = img_file.readline()
    if [int(x) for x in line.decode('ascii').split()] != [100, 100]:
        raise ValueError('eye.ppm must be a 100x100 image')
    line = img_file.readline() # 255\n
    line = img_file.read()
    img = (np.frombuffer(line, np.uint8).astype(float) / 255.0).reshape(100, 100, 3)


with cx.variable.Realize(cx.variable.path_from(root_dir, 'img', 'src')) as src:
    src[0] = img

    



