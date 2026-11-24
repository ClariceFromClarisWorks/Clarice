#include "network/command.h"
#include "../help-output.h"
int clarice_main(int argc,char **argv){
 if(argc==2&&!strcmp(argv[1],"--help"))return help_output(1,
  "Usage:\n"
  "  ipconfig\n"
  "Show the IPv4 address and subnet mask from the installed .IPP stack.\n"
  "Gateway, DNS, link and driver settings remain in the TCP/IP control panel.\n"
  "Status: 0 success, 1 stack error, 2 usage.\n");
 if(argc!=1){errsay("usage: ipconfig [--help]\n");return 2;}
 unsigned long a,m;int e=NetAddress(&a,&m);if(e){error("ipconfig: .IPP",e);return 1;}
 char ip[16],mask[16],line[160];NetFormatIP(a,ip);NetFormatIP(m,mask);
 snprintf(line,sizeof line,"Interface: .IPP (installed Macintosh TCP/IP)\nIPv4: %s\nSubnet mask: %s\n",ip,mask);say(line);
 say("Gateway/DNS/link settings: consult the installed TCP/IP control panel.\n");return output_error?1:0;
}
