import notabene as nb

# Call code
def call_code(fname, *code):
    name = nb.basics.Formula([nb.to(fname)], lambda args: '\\textcolor{funcoul}{\\mbox{' + str(args[0]) + '}}')
    return nb.fun(name)(*code)

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

T, X, t, dt, n, u = nb.to('T X t \\tau n u')
slot = nb.to('\\bullet')
any_set = nb.symbol('{\\cal X}')
DI = DIof(T, X, t)
RDI = RDIof(T, X, t)
any_time = DIof(T, X, slot)
any_var = DIof(T, slot, t)

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

def updt_arg(argname, updt):
    return nb.rm(argname)@updt

res_u = updt_arg('res', u)
in_arg_u = updt_arg('in', u)
out_arg_u = updt_arg('out', u)

with nb.files.defs('commands.tex') as defs:
    nb.config.push('display style', True) # We set the displaystyle
    defs.prefix = 'cx'

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
    defs['DftUpdt'] = u

    defs['DftTSQu'] = TSQu
    defs['DftTSQi'] = TSQi
    defs['DftTSQs'] = TSQs
    defs['DftTSQc'] = TSQc

    defs['DftTSEq'] = TS == any_var
    
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
    defs['DefVar'] = any_time
    defs['DefTimeStep'] = any_var
    defs['DefTimeStepFull'] = nb.define(TS, nb.sets.bydef(nb.sets.isin(DIof(T.prime, X.prime, t.prime), nb.text('simulation')),
                                                          nb.logical.conj(T.prime == T, t.prime == t)))
    defs['DefStatus'] = nb.sets.isin(status(DI), nb.sets.byext(busy, ready))
    defs['StatusX'] = status('x')

    defs['ResUpdt'] = res_u
    defs['InArgUpdt'] = in_arg_u
    defs['OutArgUpdt'] = out_arg_u
    defs['ResUpdtDef'] = nb.define(res_u, DI)
    defs['ResUpdtEq'] = res_u == DI

    defs['AllReady'] = nb.math.forall(nb.sets.isin(DI, out_arg_u), status(DI) == ready)
    defs['DatationDef'] = nb.sets.isin(datation(DI), nb.sets.N)
    defs['DatationResU'] = datation(res_u)
    defs['DatationUpdt'] = datation(res_u) == 1 + nb.sets.max(nb.sets.isin(DI,in_arg_u), datation(DI))
    defs['StatusUpdt']       = status(u)
    defs['StatusImpossible'] = status(u) == impossible
    defs['StatusUpToDate']   = status(u) == uptodate
    defs['StatusUpdated']    = status(u) == updated
    defs['StatusDone']       = status(u) == done
    defs['StatusNone']       = status(u) == none
    defs['ResBusy']          = status(res_u) == busy
    defs['ResReady']         = status(res_u) == ready

    out_ok = nb.text('{\\tt out\\_ok}')
    is_init = nb.text('{\\tt is\\_init}')
    is_significant = nb.to('\\alpha')
    res = nb.to('x')
    defs['CycleZ'] = call_code('on\\_computation\\_start')
    defs['CycleA'] = nb.logical.neg(out_ok)
    defs['CycleB'] = nb.sets.isin(DI, out_arg_u)
    defs['CycleC'] = status(DI) == ready
    defs['CycleD'] = call_code('on\\_read\\_out\\_arg', DI)
    defs['CycleE'] = nb.algo.affect(out_ok, False)
    defs['CycleF'] = call_code('on\\_read\\_out\\_arg\\_aborted')
    defs['CycleG'] = nb.algo.affect(out_ok, True)
    defs['CycleH'] = nb.sets.isin(DI, in_arg_u)
    defs['CycleI'] = call_code('on\\_read\\_in\\_arg', DI)
    defs['CycleJ'] = is_init
    defs['CycleK'] = nb.algo.affect(res, none)
    defs['CycleL'] = nb.algo.affect(is_significant, call_code('on\\_write\\_result', res_u))
    defs['CycleM'] = is_significant
    defs['CycleN'] = nb.algo.affect(res, updated)
    defs['CycleO'] = nb.algo.affect(res, uptodate)
    defs['CycleP'] = nb.algo.affect(res, done)
    defs['CycleQ'] = nb.algo.affect(status(res_u), ready)
    defs['CycleR'] = res

    defs['DIinTS'] = nb.sets.isin(DI, TS)

    defs['TSUpdt'] = u.bar
    defs['UUpdt'] = u.prime
    defs['UpdtPair'] = [u, u.prime]
    defs['EqUpdtPair'] = u.bar == [u, u.prime]
    defs['EqUpdtRes'] = res_u == updt_arg('res', u.prime)
    res_ub = updt_arg('res', u.bar)
    defs['ResTSUpdt'] = res_ub

    defs['StatusTS'] = status(TS)
    defs['StatusTSUpdt'] = status(u.bar)
    defs['StatusSet'] = nb.sets.isin(status(TS), nb.sets.byext(blocked, relaxing, checking, done))
    defs['StatusTSBlocked'] = status(TS) == blocked
    defs['StatusTSRelaxing'] = status(TS) == relaxing
    defs['StatusTSChecking'] = status(TS) == checking
    defs['StatusTSDone'] = status(TS) == done
    
    defs['SetStatusTSBlocked'] = nb.algo.affect(status(TS), blocked)
    defs['SetStatusTSRelaxing'] = nb.algo.affect(status(TS), relaxing)
    defs['SetStatusTSChecking'] = nb.algo.affect(status(TS), checking)
    defs['SetStatusTSDone'] = nb.algo.affect(status(TS), done)
    
    defs['StatusTSUpdtImpossible'] = status(u.bar) == impossible
    defs['StatusTSUpdtUpdated'] = status(u.bar) == updated
    defs['StatusTSUpdtDone'] = status(u.bar) == done
    defs['StatusTSUpdtUpToDate'] = status(u.bar) == uptodate
    defs['ResUpdtReady'] = res_ub == ready

    defs['TSupdtInSt'] = nb.sets.isin(u.bar, TS)

    defs['QDefs'] = nb.layout('l',
                              [TSQu, ':', nb.sets.bydef(nb.sets.isin(u.bar, TS),
                                                        nb.kat(status(u.bar), nb.text('needs to be known.')))],
                              [TSQi, ':', nb.sets.bydef(nb.sets.isin(u.bar, TS),
                                                        nb.kat(status(u.bar), nb.text('has been detected as'), impossible))],
                              [TSQs, ':', nb.sets.bydef(nb.sets.isin(u.bar, TS),
                                                        nb.kat(u.bar, nb.text('have been seen stable for the first time are here.')))],
                              [TSQc, ':', nb.sets.bydef(nb.sets.isin(u.bar, TS),
                                                        nb.kat(u.bar, nb.text('for which stability is confirmed.')))])

    defs['JobsA'] = nb.sets.isin(u.bar, nb.sets.union(TSQu, TSQs))
    defs['JobsB'] = nb.sets.isin(status(u.bar), nb.sets.byext(relaxing, checking))

    defs['ReportA'] = nb.algo.affect(TSQi, nb.sets.union(TSQi, nb.sets.byext(u.bar)))
    defs['ReportB'] = nb.algo.affect(TSQu, nb.sets.union(TSQu, nb.sets.byext(u.bar)))
    defs['ReportC'] = status(updt_arg('res', u.bar)) == ready
    defs['ReportD'] = status(TS) == checking
    defs['ReportE'] = nb.algo.affect(TSQc, nb.sets.union(TSQc, nb.sets.byext(u.bar)))
    defs['ReportF'] = nb.algo.affect(TSQs, nb.sets.union(TSQs, nb.sets.byext(u.bar)))
    defs['ReportG'] = call_code('update\\_status', TS)

    defs['UpstatA'] = TSQi != nb.sets.empty
    defs['UpstatB'] = nb.algo.affect(TSQs, nb.sets.union(TSQs, TSQc))
    defs['UpstatC'] = nb.algo.affect(TSQc, nb.sets.empty)
    defs['UpstatD'] = nb.algo.affect(TSQu, nb.sets.union(TSQu, TSQi))
    defs['UpstatE'] = nb.algo.affect(TSQi, nb.sets.empty)
    TTS = time_step@(T.prime, t.prime)
    defs['UpstatF'] = TTS
    defs['UpstatG'] = TSQu != nb.sets.empty
    defs['UpstatH'] = TSQi == nb.sets.empty
    defs['UpstatI'] = nb.algo.affect(TSQs, nb.sets.union(TSQs, TSQc))
    defs['UpstatJ'] = nb.algo.affect(TSQc, nb.sets.empty)
    defs['UpstatK'] = TSQs != nb.sets.empty
    defs['UpstatL'] = nb.seq(TSQi == nb.sets.empty, TSQu == nb.sets.empty)
    defs['UpstatM'] = nb.seq(TSQi == nb.sets.empty, TSQu == nb.sets.empty, TSQs == nb.sets.empty)
    defs['UpstatN'] = nb.sets.isin(u.bar, TSQc)
    defs['UpstatO'] = nb.algo.affect(status(res_u), ready)
    defs['UpstatP'] = call_code('update\\_status', TTS)
    
    defs.add_preamble('\\usepackage{xspace}')
    defs.add_preamble('\\usepackage{color}')
    defs.add_preamble('\\input{globalcommands.tex}')
    defs.cheatsheet()
