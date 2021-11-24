import notabene as nb

# Call code
def call_code(fname, code):
    name = nb.basics.Formula([nb.to(fname)], lambda args: '\\textcolor{funcoul}{\\mathtt{ ' + str(args[0]) + '}}')
    return nb.fun(name)(code)

def tag(tagname):
    return nb.basics.Formula([], lambda args : '\\textcolor{statuscoul}{\\mathtt{' + tagname + '}}')

busy  = tag('busy')
ready = tag('ready')

impossible = tag('impossible')
uptodate   = tag('uptodate')
updated    = tag('updated')
done       = tag('done')
none       = tag('none')

blocked  = tag('blocked')
relaxing = tag('relaxing')
checking = tag('checking')

def DIof(T, X, t):
    return nb.math.matrix(nb.seq(T, X, t))
def RDIof(T, X, t):
    return nb.math.brace(nb.seq(T, X, t))

T, X, t, dt, n = nb.to('T X t \\tau n')
slot = nb.to('\\bullet')
any_set = nb.symbol('{\\cal X}')
DI = DIof(T, X, t)
RDI = RDIof(T, X, t)
any_var = DIof(T, X, slot)

time_step = nb.symbol('{\\cal S}')
TS = time_step@(T, t)

def TSQ(ts, queue):
    return nb.math.matrix(ts)@nb.rm(queue)
TSQu = TSQ(TS, 'unstable')
TSQi = TSQ(TS, 'impossible')
TSQs = TSQ(TS, 'stable')
TSQc = TSQ(TS, 'confirmed')

def datation(x):
    return nb.rm('d')@nb.to(x)

def status(x):
    return nb.math.bracket(x)

with nb.files.defs('commands.tex') as defs:
    nb.config.push('display style', True) # We set the displaystyle
    defs.prefix = 'cx'

    defs['CallCode'] = call_code(nb.arg(1), nb.arg(2))

    defs['Busy']       = busy
    defs['Ready']      = ready
    defs['Impossible'] = impossible
    defs['UpToDate']   = uptodate
    defs['Updated']    = updated
    defs['Done']       = done
    defs['None']       = none
    defs['Blocked']    = blocked
    defs['Relaxing']   = relaxing
    defs['Checking']   = checking

    defs['DftVar'] = X
    defs['DftTL'] = T
    defs['DftInst'] = t
    defs['DftDI'] = DI
    defs['DftRDI'] = RDI
    defs['DftTS'] = TS

    defs['DftTSQu'] = TSQu
    defs['DftTSQi'] = TSQi
    defs['DftTSQs'] = TSQs
    defs['DftTSQc'] = TSQc

    defs['Rn'] = nb.sets.R**n
    defs['ArraySet'] = nb.seq(nb.sets.range_cc(0, 1)**n, nb.sets.isin(n, nb.sets.N.star))
    defs['AnySet'] = any_set
    defs['ContentSetD'] = nb.seq(any_set**n, nb.sets.isin(n, nb.sets.N.star))
    defs['ContentSetDD'] = nb.seq([any_set**n]**n, nb.sets.isin(n, nb.sets.N.star))
    defs['AnySetDef'] = nb.sets.isin(any_set, nb.sets.byext(nb.text('\\Scalar'),
                                                            nb.text('\\Pos 1'),
                                                            nb.text('\\Pos 2'),
                                                            nb.text('\\Array k')))

    defs['DefDatation'] = nb.sets.isin(datation(DI), nb.sets.N)
    defs['DefVar'] = any_var
    defs['DefStatus'] = nb.sets.isin(status(DI), nb.sets.byext(busy, ready))
    defs['StatusX'] = status('x')

    defs.add_preamble('\\usepackage{xspace}')
    defs.add_preamble('\\usepackage{color}')
    defs.add_preamble('\\definecolor{funcoul}{rgb}{0,0,0.75}')
    defs.add_preamble('\\definecolor{statuscoul}{rgb}{0,0.5,0}')
    defs.add_preamble('\\newcommand{\\Scalar}[0]{{\\tt Scalar}\\xspace}')
    defs.add_preamble('\\newcommand{\\Pos}[1]{{\\tt Pos{#1}D}\\xspace}')
    defs.add_preamble('\\newcommand{\\Array}[1]{{{\\tt Array=}$#1$}\\xspace}')
    defs.add_preamble('\\newcommand{\\Map}[3]{{{\\tt{Map{#1}D<}{$#2$}{>=}$#3$}}\\xspace}')
    defs.cheatsheet()
