"""Temporary peer-allowlisted Telnet-to-PTY acceptance bridge.
Reuses serial-shell's restricted Bash fixture; restriction is NOT a sandbox.
No login credentials, persistent service or access to Clarice's shell.
"""
import argparse, asyncio, errno, json, os, pty, signal, tempfile, termios, time, fcntl, struct
from pathlib import Path
import telnetlib3
from telnetlib3.telopt import IP
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--bind',required=True);p.add_argument('--peer',action='append',required=True)
p.add_argument('--port',type=int,default=2323);p.add_argument('--seconds',type=int,default=180)
a=p.parse_args();active=False

def record(event,**kw):print(json.dumps(dict(t=time.monotonic(),event=event,**kw)),flush=True)
class Server(telnetlib3.TelnetServer):
    def connection_made(self,transport):
        if transport.get_extra_info('peername')[0] not in a.peer:transport.close();return
        super().connection_made(transport)

async def session(reader,writer):
    global active
    if active:writer.close();return
    active=True;child=None;master=None;tasks=[]
    try:
        with tempfile.TemporaryDirectory(prefix='clarice-telnet-') as scratch:
            root=Path(scratch);tools=root/'bin';tools.mkdir()
            for name,source in {'ls':'/bin/ls','cat':'/bin/cat','sleep':'/bin/sleep','wc':'/usr/bin/wc','head':'/usr/bin/head','vi':'/usr/bin/vi','less':'/usr/bin/less','stty':'/bin/stty'}.items():
                (tools/name).symlink_to(source)
            (root/'sample.txt').write_text('remote fixture\n')
            (root/'lines.txt').write_text(''.join('line %03d ABCDEFGHIJKLMNOPQRSTUVWXYZ\n'%i for i in range(120)))
            term=writer.get_extra_info('TERM','dumb').lower();term=term if term in ('vt100','dumb') else 'dumb'
            rows=int(writer.get_extra_info('rows',24));cols=int(writer.get_extra_info('cols',80))
            child,master=pty.fork()
            if child==0:
                os.chdir(root);fcntl.ioctl(0,termios.TIOCSWINSZ,struct.pack('HHHH',rows,cols,0,0));attrs=termios.tcgetattr(0);attrs[6][termios.VERASE]=b'\x7f'
                termios.tcsetattr(0,termios.TCSANOW,attrs)
                os.execve('/bin/bash',['bash','--noprofile','--norc','--restricted','--noediting','-i'],
                    {'TERM':term,'BASH_SILENCE_DEPRECATION_WARNING':'1','LANG':'C','LC_ALL':'C','HOME':scratch,'PATH':str(tools),'PS1':'MACBOOK> ','PS2':'MORE> ','HISTFILE':'/dev/null'})
            os.set_blocking(master,False);record('start',pid=child,term=term,rows=rows,cols=cols)
            def interrupt(cmd):
                os.write(master,b'\x03');record('interrupt')
            writer.set_iac_callback(IP,interrupt)
            async def incoming():
                cr=False
                while True:
                    text=await reader.read(256)
                    if not text:return
                    record('input',text=text);data=bytearray()
                    for c in text:
                        if cr and c in '\n\x00':cr=False;continue
                        cr=c=='\r';data.append(ord(c))
                    while data:
                        try:n=os.write(master,data);del data[:n]
                        except BlockingIOError:await asyncio.sleep(.01)
            async def outgoing():
                while True:
                    try:data=os.read(master,1024)
                    except BlockingIOError:await asyncio.sleep(.01);continue
                    except OSError as e:
                        if e.errno==errno.EIO:return
                        raise
                    if not data:return
                    record('output',text=data.decode('ascii','replace'))
                    writer.write(data.decode('ascii','replace'));await writer.drain()
            tasks=[asyncio.create_task(incoming()),asyncio.create_task(outgoing())]
            done,_=await asyncio.wait(tasks,timeout=a.seconds,return_when=asyncio.FIRST_COMPLETED)
            for task in done:task.result()
            record('end',reason='closed' if done else 'timeout')
            # Reap before deleting the disposable working directory.
            for task in tasks:task.cancel()
            await asyncio.gather(*tasks,return_exceptions=True)
            if master is not None:os.close(master);master=None
            if child:
                try:os.kill(child,signal.SIGHUP)
                except ProcessLookupError:pass
                for _ in range(100):
                    got,status=os.waitpid(child,os.WNOHANG)
                    if got:break
                    await asyncio.sleep(.01)
                if not got:os.kill(child,signal.SIGKILL);_,status=os.waitpid(child,0)
                record('reaped',pid=child,status=status);child=None
            record('saved-file',text=(root/'sample.txt').read_text(errors='replace')[:1024])
    finally:
        for task in tasks:task.cancel()
        if master is not None:os.close(master)
        if child:
            try:os.kill(child,signal.SIGKILL)
            except ProcessLookupError:pass
            os.waitpid(child,0)
        writer.close();active=False;record('cleanup')
async def main():
    server=await telnetlib3.create_server(a.bind,a.port,protocol_factory=Server,shell=session,encoding='ascii',connect_maxwait=10,timeout=a.seconds+5,limit=8192,compression=False)
    record('listening',address=a.bind,port=a.port)
    await server.wait_closed()
asyncio.run(main())
