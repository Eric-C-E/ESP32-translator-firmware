```python
import socket

HOST = "192.168.0.165" #HOST
PORT = 3333

#define socket and bind socket

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
	s.bind((HOST, PORT))
	s.listen(1)
	s.setblocking(false)
```

AF_INET is IPV4
SOCK_STREAM is TCP

python with object as name:
	name.function

bind has different arguments for each type of connection. IPV4 expects a two-tuple (host, port).

setsockopt on sol_socket means socket - level ops, setting SO_REUSEADDR to 1 (True) which allows it to use addresses that have been previously "claimed".