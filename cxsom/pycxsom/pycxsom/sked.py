import socket


class _locker:
    def __init__(self, server_tag, client_tag, hostname, port):
        self.server_tag = server_tag
        self.client_tag = client_tag+'\n'
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.connect((hostname, port))
        
    def _interact(self):
        self.s.sendall(bytes(self.client_tag, encoding='utf-8'))
        buf = self.s.recv(1024).decode("utf-8")
        tag = buf[0]
        if len(buf) < 2: # buf should be 'X\n'... but sometimes, I get 'X' only, and '\n' stays in the stream
            self.s.recv(1024) # I flush the remaining byte.
        if tag != self.server_tag:
            raise ValueError('sked protocol error : '+line)
    
    def __enter__(self):
        self._interact()
        return self
    
    def __exit__(self, exc_type, exc_value, exc_traceback):
        self._interact()
        if exc_type:
            print(f'exc_type: {exc_type}')
            print(f'exc_value: {exc_value}')
            print(f'exc_traceback: {exc_traceback}')

class write(_locker):
    def __init__(self, hostname, port):
        super().__init__('W', 'w', hostname, port)

class read(_locker):
    def __init__(self, hostname, port):
        super().__init__('R', 'r', hostname, port)

class nolock:
    def __init__(self):
        pass
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_value, exc_traceback):
        if exc_type:
            print(f'exc_type: {exc_type}')
            print(f'exc_value: {exc_value}')
            print(f'exc_traceback: {exc_traceback}')
