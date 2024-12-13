import pycxsom as cx

type_descriptions = ['',                     # [EXCEPT] <- '' : Parse error : make('') : Empty type description
                     'Skalar',               # [EXCEPT] <- 'Skalar' : Parse error : parse_check_next('Skalar', ...) : 'Scalar' expected.
                     'Scalar',               # [  OK  ] <- 'Scalar'
                     'Pos1D',                # [  OK  ] <- 'Pos1D'
                     'Pos2D',                # [  OK  ] <- 'Pos2D'
                     'Pos3D',                # [EXCEPT] <- 'Pos3D' : Parse error : parse_DIM('3D') : This is not 1D or 2D.
                     'Array=10',             # [  OK  ] <- 'Array=10'
                     'Mop1D<Scalar>=100',    # [EXCEPT] <- 'Mop1D<Scalar>=100' : Parse error : parse_check_next('Mop1D<Scalar>=100', ...) : 'Map' expected.
                     'Map1D<Scalar>=100',    # [  OK  ] <- 'Map1D<Scalar>=100'
                     'Map1D<Pos1D>=100',     # [  OK  ] <- 'Map1D<Pos1D>=100'
                     'Map1D<Pos2D>=100',     # [  OK  ] <- 'Map1D<Pos2D>=100'
                     'Map1D<Array=10>=100',  # [  OK  ] <- 'Map1D<Array=10>=100'
                     'Map2D<Scalar>=100',    # [  OK  ] <- 'Map2D<Scalar>=100'
                     'Map2D<Pos1D>=100',     # [  OK  ] <- 'Map2D<Pos1D>=100'
                     'Map2D<Pos2D>=100',     # [  OK  ] <- 'Map2D<Pos2D>=100'
                     'Map2D<Array=10>=100',  # [  OK  ] <- 'Map2D<Array=10>=100'
                     'Map3D<Array=10>=100']  # [EXCEPT] <- 'Map3D<Array=10>=100' : Parse error : parse_DIM('3D<Array=10>=100') : This is not 1D or 2D.

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
            
    
