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

T, X, t, dt = nb.to('T X t \\tau')
DI = DIof(T, X, t)
RDI = RDIof(T, X, t)

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

    defs['DftDI'] = DI
    defs['DftRDI'] = RDI
    defs['DftTS'] = TS

    defs['DftTSQu'] = TSQu
    defs['DftTSQi'] = TSQi
    defs['DftTSQs'] = TSQs
    defs['DftTSQc'] = TSQc

    defs['DefDatation'] = nb.sets.isin(datation(DI), nb.sets.N)

    defs.add_preamble('\\usepackage{color}')
    defs.add_preamble('\\definecolor{funcoul}{rgb}{0,0,0.75}')
    defs.add_preamble('\\definecolor{statuscoul}{rgb}{0,0.5,0}')
    defs.cheatsheet()
