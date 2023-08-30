from pathlib import Path

class Monitor:
    def __init__(self, filepath=Path('monitoring.data')):
        self.filepath = Path(filepath)
        self.datafile = open(self.filepath, 'r')
        self.checkpoint = None
        self.checkpoint_head = None
        self.goto(0)

    def __repr__(self):
        res = '\n\n\nhelp\n----\n\n'
        res += 'm = Monitor()\n\n'
        res += 'm.goto(2), m += 3, m -= 1, m.position(), m.goto() : handle checkpoint positioning.'
        res += '\n\n'
        return res
        

    def goto(self, checkpoint = None):
        if not isinstance(checkpoint, int) or checkpoint < 0:
            self.goto(0)
            return
        if checkpoint is not None:
            self.checkpoint = checkpoint
            self.datafile.seek(0)
            while True:
                line = self.datafile.readline()
                if line == '':
                    break
                if line[0] == '#':
                    cp = int(line.split()[1])
                    self.checkpoint_head = self.datafile.tell()
                    if cp == checkpoint:
                        self.checkpoint = cp
                        break
                    self.checkpoint = cp
        else:
            self.datafile.seek(self.checkpoint_head)

    def __len__(self):
        """
        Returns: The number of checkpoints.
        
        Warning: After this, the next line is the one just next to the position before len was called.
        """
        curr = self.checkpoint
        cp = self.checkpoint
        last = cp - 1
        while cp != last:
            last = cp
            self += 1000
            cp =  self.position()
        res = cp
        self.goto(curr)
        return res+1

    def __iadd__(self, value):
        self.goto(self.checkpoint + value)
        return self
    
    def __isub__(self, value):
        self.goto(self.checkpoint - value)
        return self
    
    def position(self):
        return self.checkpoint

    def next_line(self):
        line = self.datafile.readline()
        if line == '':
            return None
        return line

    def next(self):
        line = self.next_line()
        if line == None:
            return line
        line = line.split()
        info = line[0]
        if info == '#':
            return {'type': 'checkpoint', 'info': int(line[1])}
        elif info == 'timestep':
            ts = Monitor._parse_timestep(line[1])
            return self._parse_timestep_info(ts, line[2:])
        elif info == 'job':
            return self._parse_job_info(line[1:])
        else:
            raise ValueError(f'ParseError: invalid info ({info}).')

    def __iter__(self):
        return self

    def __next__(self):
        data = self.next()
        if data == None:
            raise StopIteration
        return data
        
    def _parse_timestep_info(self, timestep, line):
        info = line[0]
        res = {'type': 'timestep', 'info': info, 'timestep': timestep}
        if info == 'launch':
            res['status'] = Monitor._parse_status(line[1])
            return res
        if info == 'terminated':
            res['why'] = line[1]
            return res
        if info == 'add':
            res['var'] = line[1]
            return res
        if info == 'dont-add':
            res['var'] = line[1]
            return res
        if info == 'update':
            res['why'] = line[1]
            res['status'] = Monitor._parse_status(line[2])
            line = line[3:]
            line = Monitor._parse_queue(res, line, Monitor._parse_varname)
            line = Monitor._parse_queue(res, line, Monitor._parse_varname)
            line = Monitor._parse_queue(res, line, Monitor._parse_varname)
            line = Monitor._parse_queue(res, line, Monitor._parse_varname)
            line = Monitor._parse_queue(res, line, Monitor._parse_varname)
            line = Monitor._parse_queue(res, line, Monitor._parse_timestep)
            line = Monitor._parse_queue(res, line, Monitor._parse_varname)
            return res
        if info == 'report':
            res['var'] = line[1]
            res['status'] = Monitor._parse_status(line[2])
            return res
        raise ValueError(f'ParseError: invalid info ({info}).')
        
    
    def _parse_job_info(self, line):
        info = line[0]
        res = {'type': 'job', 'info': info}
        if info == 'out-of-tasks':
            res['why'] = line[1]
            return res
        if info == 'tasks':
            res['tasks'] = [Monitor._parse_instance(inst) for inst in line[2:]]
            return res      
        if info == 'execute':
            res['task'] = Monitor._parse_instance(line[1])
            return res
        raise ValueError(f'ParseError: invalid info ({info}).')

    def _parse_varname(varname):
        return varname
        
    def _parse_timestep(ts):
        timeline, at = (ts[2: -1]).split(',')
        return {'timeline': timeline, 'at': int(at)}
    
    def _parse_instance(inst):
        timeline, varname, at = (inst[2: -1]).split(',')
        return {'timeline': timeline, 'var': varname, 'at': int(at)}
    
    def _parse_status(status):
        return status[1:-1]

    def _parse_queue(res, line, converter):
        name = line[0]
        nb = int(line[1])
        data = line[2:]
        res[name] = [converter(d) for d in data[:nb]]
        return data[nb:]
    
