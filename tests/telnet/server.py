"""Peer-restricted Telnet interoperability fixture (telnetlib3 5.0.2).
TEST> is a test service, not the MacBook or Clarice shell. Executes no programs.
"""
import argparse,asyncio,json,time
import telnetlib3
from telnetlib3.telopt import IP
p=argparse.ArgumentParser();p.add_argument('--bind',required=True);p.add_argument('--peer',action='append',required=True);p.add_argument('--port',type=int,default=2323);p.add_argument('--auto-close',action='store_true');a=p.parse_args()
def record(event,**kw):print(json.dumps(dict(t=time.monotonic(),event=event,**kw)),flush=True)
class Server(telnetlib3.TelnetServer):
    def connection_made(self,transport):
        if transport.get_extra_info('peername')[0] not in a.peer:transport.close();return
        super().connection_made(transport)
async def session(reader,writer):
    record('connected',term=writer.get_extra_info('TERM'))
    line=[];cr=False
    def interrupt(cmd):
        line.clear();record('interrupt');writer.write('\r\nREMOTE INTERRUPT\r\nTEST> ')
    writer.set_iac_callback(IP,interrupt)
    writer.write('\r\nTELNET FIXTURE: echo TEXT, burst, wait, close\r\nTEST> ')
    if a.auto_close:
        writer.write('AUTO_DONE\r\n');await writer.drain();writer.close();record('auto-close');return
    try:
        while True:
            c=await reader.read(1)
            if not c:break
            record('key',value=ord(c))
            if c=='\n' and cr:cr=False;continue
            if c=='\x00' and cr:cr=False;continue
            if c in '\r\n':
                cr=c=='\r';cmd=''.join(line);line.clear();writer.echo('\r\n');record('command',text=cmd)
                if cmd=='close':break
                if cmd.startswith('echo '):writer.write(cmd[5:]+'\r\n')
                elif cmd=='burst':writer.write(''.join('row %03d ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n'%i for i in range(100)))
                elif cmd=='wait':continue
                else:writer.write('fixture: unknown command\r\n')
                writer.write('TEST> ')
            elif c in '\x7f\x08':
                if line:line.pop();writer.echo('\b \b')
            else:
                cr=False
                if len(line)>=4096:writer.write('\r\nfixture input limit\r\n');break
                line.append(c);writer.echo(c)
            await writer.drain()
    finally:writer.close();record('closed')
async def main():
    server=await telnetlib3.create_server(a.bind,a.port,protocol_factory=Server,shell=session,encoding='ascii',connect_maxwait=1,timeout=120,limit=8192,compression=False)
    record('listening',address=a.bind,port=a.port)
    await server.wait_closed()
asyncio.run(main())
