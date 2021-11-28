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

unbound  = tag('unbound')
blocked  = tag('blocked')
relaxing = tag('relaxing')
checking = tag('checking')

def DIof(T, X, t):
    return nb.math.matrix(nb.seq(T, X, t))
def RDIof(T, X, t):
    return nb.math.brace(nb.seq(T, X, t))

T, X, t, dt, n, u, size, size_of = nb.to('T X t \\tau n u s d')
pat = nb.to('\\pi')
pats = nb.symbol('{\\cal P}')

slot = nb.to('\\bullet')
any_set = nb.symbol('{\\cal X}')
DI = DIof(T, X, t)
RDI = RDIof(T, X, dt)
any_time = DIof(T, X, slot)
any_var = DIof(T, slot, t)

time_step = nb.symbol('{\\cal S}')
TS = time_step@(T, t)
time_step_update = nb.symbol('{\\cal U}')
TSU = time_step_update@(T, t)
time_step_out = nb.symbol('{\\cal O}')
TSO = time_step_out@(T, t)
time_step_free = nb.symbol('{\\cal F}')
TSF = time_step_free@(T, t)

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

def min_t(timeline):
    return nb.cat(nb.text('\\tt @'), timeline)
minT = min_t(T)

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
    defs['Unbound']    = unbound
    defs['Blocked']    = blocked
    defs['Relaxing']   = relaxing
    defs['Checking']   = checking

    defs['DftVar'] = X
    defs['DftTL'] = T
    defs['DftInst'] = t
    defs['DftDI'] = DI
    DDI = DIof(T.prime, X.prime, t.prime)
    defs['DftDDI'] = DIof(T, X, t.prime)
    defs['DftDIz'] = DIof(T, X, 0)
    defs['BufSize'] = size
    defs['BufFirst'] = DIof(T, X, t - size  + 1)
    defs['BufLast'] = DIof(T, X, t + 1)
    defs['Sizeof'] = size_of
    defs['DataSize'] = size^[size_of+1]
    defs['DftRDI'] = RDI
    defs['DftTS'] = TS
    defs['DftUpdt'] = u
    defs['DftShift'] = dt

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
    defs['StatusSet'] = nb.sets.isin(status(TS), nb.sets.byext(unbound, blocked, relaxing, checking, done))
    defs['StatusTSUnbound'] = status(TS) == unbound
    defs['StatusTSBlocked'] = status(TS) == blocked
    defs['StatusTSRelaxing'] = status(TS) == relaxing
    defs['StatusTSChecking'] = status(TS) == checking
    defs['StatusTSDone'] = status(TS) == done
    
    defs['SetStatusTSUnbound'] = nb.algo.affect(status(TS), unbound)
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

    defs['UbdA'] = call_code('has\\_unbound', TS)
    defs['UbdB'] = TSF != nb.sets.empty

    defs['ShiftSet'] = nb.sets.isin(dt, nb.sets.Z)
    defs['Anchor'] = DIof(T, X, t+dt)
    defs['Pat'] = pat
    defs['Patterns'] = pats
    res_pat = updt_arg('res', pat)
    RDIz = RDIof(T, X, 0)
    defs['ResPat'] = res_pat
    defs['RDIz'] = RDIz
    defs['ResPatIsRDIz'] = res_pat == RDIz

    jobs = nb.symbol('{\\cal J}')
    job  = nb.to('j')
    defs['Jobs'] = jobs
    defs['Job'] = job
    defs['SimA'] = call_code('get\\_one\\_job')
    defs['SimB'] = jobs == nb.sets.empty
    defs['SimC'] = nb.sets.isin(status(TS), nb.sets.byext(relaxing, checking))
    defs['SimD'] = call_code('push', jobs, nb.sets.union(TSQu, TSQs))
    defs['SimE'] = call_code('anchor\\_UPs')
    defs['SimF'] = nb.algo.affect(job, call_code('pop', jobs))

    defs['AnchorA'] = call_code('anchor\\_UPs')
    variables = nb.symbol('{\\cal V}')
    defs['Variables'] = variables
    defs['AnchorB'] = nb.define(variables, nb.sets.bydef([T, X],nb.logical.conj(nb.sets.isin(pat, pats), res_pat == RDI)))
    defs['MinT'] = minT
    defs['AnchorC'] = nb.sets.isin([T, X], variables)
    defs['AnchorD'] = nb.sets.isin(pat, pats)
    defs['AnchorE'] = nb.equal(RDIz, res_pat)
    defs['AnchorF'] = status(DIof(T, X, minT)) == busy


    #### Learning rules
    xi, sigma, w, i, r, a, g, e, c, h, t, mu, alpha, beta = nb.to('\\xi \\sigma w i r a g e c h t \\mu \\alpha \\beta')
    bmu = nb.to('\\pi')
    wi = w@i
    ai = a@i
    match_triangle = mu@'\\triangle'
    match_gaussian = mu@'G'
    match = nb.fun(mu)
    defs['Xi'] = xi
    defs['Wi'] = wi
    defs['Ai'] = ai
    defs['MatchG'] = match_gaussian
    defs['MatchT'] = match_triangle
    defs['AiDef'] = nb.math.forall(i, ai == match(xi, wi))
    defs['BMU'] = bmu

    match_G_def = nb.define(nb.fun(match_gaussian)(xi, w), nb.math.exp(nb.to([xi - w])**2/(2*sigma**2)))
    match_T_def = nb.define(nb.fun(match_triangle)(xi, w), nb.math.max(1-nb.math.abs(xi - w)/r, 0))
    defs['MatchEq'] = nb.layout('l', [match_G_def], [match_T_def])
    defs['MergeEq'] = nb.math.forall(i, a@(i, g) == nb.math.sqrt(a@(i, e) + [1 - beta]*a@(i, c)))
    topomatch = nb.fun(mu)(i, bmu)
    defs['TopoMatch'] = topomatch
    defs['LearnEq'] = nb.seq(nb.math.forall(i, w@(i, t+1) == [1-alpha*h]*w@(i,t) + alpha*h*xi),
                             nb.kat(nb.text('with'), h == topomatch))
    


    defs['TSU'] = TSU
    defs['TSUDef'] = nb.define(TSU, nb.sets.bydef(u, nb.sets.isin(res_u, TS)))
    defs['TSO'] = TSO
    defs['TSODef'] = nb.define(TSO, nb.sets.bydef(DI,
                                                  nb.math.exists(nb.sets.isin(u, TSU), nb.sets.isin(DI, out_arg_u))))
    act1 = nb.math.forall(nb.sets.isin(DDI, TSO), status(DDI) == ready)
    defs['TSOAct'] = act1
    
    defs['TSF'] = TSF
    defs['TSFDef'] = nb.define(TSF, nb.sets.minus(TS, nb.sets.bydef(res_u, nb.sets.isin(u, TSU))))
    DDIX = DIof(T, X.prime, t)
    act2 = nb.math.forall(nb.sets.isin(DDIX, TSF), status(DDIX) == ready)
    defs['TSFAct'] = act2
    defs['TSAct'] = nb.logical.conj([act1], [act2])
    defs['HasTSF'] = (TSF != nb.sets.empty)
    
    defs.add_preamble('\\usepackage{xspace}')
    defs.add_preamble('\\usepackage{color}')
    defs.add_preamble('\\input{globalcommands.tex}')
    defs.cheatsheet()
