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

    defs.add_preamble('\\usepackage{color}')
    defs.add_preamble('\\definecolor{funcoul}{rgb}{0,0,0.75}')
    defs.add_preamble('\\definecolor{statuscoul}{rgb}{0,0.5,0}')
    defs.cheatsheet()
