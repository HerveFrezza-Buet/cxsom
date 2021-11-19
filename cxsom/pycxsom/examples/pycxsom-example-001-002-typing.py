import pycxsom as cx

type_descriptions = ['',
                     'Skalar',
                     'Scalar',
                     'Pos1D',
                     'Pos2D',
                     'Pos3D',
                     'Array=10',
                     'Mop1D<Scalar>=100',
                     'Map1D<Scalar>=100',
                     'Map1D<Pos1D>=100',
                     'Map1D<Pos2D>=100',
                     'Map1D<Array=10>=100',
                     'Map2D<Scalar>=100',
                     'Map2D<Pos1D>=100',
                     'Map2D<Pos2D>=100',
                     'Map2D<Array=10>=100',
                     'Map3D<Array=10>=100']

for descr in type_descriptions:
    msg = "[  OK  ]"
    error = ''
    try:
        t = cx.typing.make(descr)
        if descr != str(t):
            msg = "[FAILED]"
    except cx.error.Any as err:
            msg = "[EXCEPT]"
            error = str(err)
    display = "{} <- '{}'".format(msg, descr)
    if msg == "[FAILED]":
        display += " instead of " + str(t)
    elif msg == "[EXCEPT]":
        display += " : " + str(error)
    print(display)
            
    
