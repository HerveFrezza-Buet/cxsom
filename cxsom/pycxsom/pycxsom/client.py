import socket

def ping(hostname, port):
    """
    returns None is ping has been sent, an error string otherwise.
    """
    line = 'Connection error'
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((hostname, port))
        s.sendall(b'ping\n')
        line = s.recv(1024).decode("utf-8").split('\n')[0]
    if line == 'ok':
        return None
    return line
    

