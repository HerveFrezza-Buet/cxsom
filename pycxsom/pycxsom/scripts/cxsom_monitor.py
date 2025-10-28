#!/usr/bin/env python
# coding: utf-8

import pycxsom as cx
import numpy as np
import sys
from pathlib import Path

def title(msg):
    print()
    print(f'# {msg}')
    print()
    
def section(msg):
    print()
    print(f'## {msg}')
    print()
    
def subsection(msg):
    print()
    print(f'### {msg}')
    print()
    
def subsubsection(msg):
    print()
    print(f'### {msg}')
    print()
    
def report_main(monitor):
    title('Monitoring report')
    section('Main information')
    print(f'nb checkpoints: {len(monitor)}.')
    print()
    print(f'`S(T, t)` stands for the timestep in timeline `T`, at step `t`.')

class TimeLineInfo:
    def __init__(self):
        self.min_step = None
        self.max_step = None
        self.variables = set()
        
def report_timelines(monitor):
    section('Timelines')
    monitor.goto(0)
    timelines = {}
    for data in monitor:
        if data['type'] == 'timestep' and data['info'] in ['add', 'dont-add']:
            ts = data['timestep']
            timeline, at = ts['timeline'], ts['at']
            if timeline not in timelines:
                timelines[timeline] = TimeLineInfo()
            tli = timelines[timeline]
            if tli.min_step == None:
                tli.min_step = at
                tli.max_step = tli.min_step
            elif tli.max_step < at:
                tli.max_step = at
            tli.variables.add(data['var'])
    for timeline, tli in timelines.items():
        print(f'- `{timeline}`:')
        print(f'\t- t in [{tli.min_step}, {tli.max_step}]')
        variables = ', '.join([f'`{name}`' for name in sorted(tli.variables)])
        print(f'\t- variables: {variables}')

def report_walltime_issues(monitor):
    monitor.goto(0)
    issues = set()
    for data in monitor:
        if data['type'] == 'timestep':
            info = data['info']
            if info in ['add', 'dont-add']:
                ts = data['timestep']
                timeline, at = ts['timeline'], ts['at']
                varname = data['var']
                elem = (timeline, varname, at)
                if info == 'add':
                    try:
                        issues.remove(elem)
                    except KeyError:
                        pass
                else:
                    issues.add(elem)
    section('Walltime issues')
    print('Here are the data instances for which an update pattern is defined, but not applied due to pattern walltime limitation.')

    timelines = set()
    for (timeline, varname, at) in issues:
        timelines.add(timeline)
    timelines = sorted(list(timelines))
    timelines = {tl: {} for tl in timelines}
    for tl in timelines:
        ats = set()
        for (timeline, varname, at) in issues:
            if timeline == tl:
                ats.add(at)
        ats = sorted(list(ats))
        for a in ats:
            vs = set()
            for (timeline, varname, at) in issues:
                if timeline == tl and at == a:
                    vs.add(varname)
            vs = sorted(list(vs))
            for v in vs:
                print(f'- `S({tl}, {a})`: `{v}`')
            
        
    
def report_pending_timesteps(monitor):
    monitor.goto(0)
    timelines = {}
    for data in monitor:
        if data['type'] == 'timestep':
            info = data['info']
            ts = data['timestep']
            timeline, at = ts['timeline'], ts['at']
            if info in ['launch', 'terminated']:
                if info == 'launch':
                    if timeline not in timelines:
                        timelines[timeline] = {}
                    tli = timelines[timeline]
                    if at not in tli:
                        tli[at] = {}
                elif info == 'terminated':
                    if timeline not in timelines:
                        raise RuntimeError(f'Termination of a timestep which is not launched (checkpoint={monitor.position}, data={data})')
                    tli = timelines[timeline]
                    if at not in tli:
                        raise RuntimeError(f'Termination of a timestep which is not launched (checkpoint={monitor.position}, data={data})')
                    tli.pop(at)
            elif info == 'update':
                timelines[timeline][at] = data
    section('Pending timesteps')
    
    for timeline, dic in timelines.items():
        for at, data in dic.items():
            print()
            lists = ['new', 'unstable', 'impossible', 'stable', 'confirmed']
            infos = ['blockers', 'unbounds']
            nb_elem_max = max([len(data[l]) for l in (lists + infos)])
            
            header = f'| **`S({data["timestep"]["timeline"]},{data["timestep"]["at"]})`**, [{data["status"]}] |'
            header += '     |'*nb_elem_max
            print(header)
            hline = '|---:|'
            hline += '----|'*nb_elem_max
            print(hline)
            
            for l in lists:
                line = f'| {l} |'
                for elem in data[l]:
                    line += f' `{elem}` |'
                print(line)
                
            line = '| **blockers** (issue with "out" variables) |'
            for elem in data['blockers']:
                line += f' `S({elem["timeline"]}, {elem["at"]})` |'
            print(line)
                
            line = '| **unbounds** (issue with "in" variables) |'
            for elem in data['unbounds']:
                line += f' `{elem}` |'
            print(line)

            
            print()
            
                    
    
    
    
       
def main():
    if len(sys.argv) > 2:
        print(f'usage : {sys.argv[0]} [monitoring-data-file]')
        sys.exit(0)

    if len(sys.argv) == 2:
        monitor = cx.monitor.Monitor(Path(sys.argv[1]))
    else:
        monitor = cx.monitor.Monitor()
    
    report_main(monitor)
    report_timelines(monitor)
    report_walltime_issues(monitor)
    report_pending_timesteps(monitor)



print()
print()
