#!/usr/bin/env python3
"""Local Snow PTY bridge to a restricted, disposable macOS test shell.
No TCP listener, login service, or access to Clarice's own shell.
Restricted bash is a convenience restriction, NOT a filesystem security sandbox.
"""
import argparse, errno, json, os, pty, select, signal, stat, subprocess, sys, tempfile, termios, time, tty
from pathlib import Path

LIMIT = 4096

def serve(device, seconds, log):
    # Explicitly accept only local PTY devices owned by the current account.
    if not str(device).startswith('/dev/ttys'):
        raise ValueError('use the PTY path printed by Snow (/dev/ttys...); no TCP or physical ports')
    st = os.stat(device)
    if not stat.S_ISCHR(st.st_mode) or st.st_uid != os.getuid():
        raise ValueError('PTY must be a character device owned by the current account')
    fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    original = termios.tcgetattr(fd)
    tty.setraw(fd)
    child = None
    master = None
    start = time.monotonic()
    def record(event, **data):
        log.write(json.dumps(dict(t=time.monotonic()-start, event=event, **data))+'\n');log.flush()
    try:
        with tempfile.TemporaryDirectory(prefix='clarice-remote-') as scratch:
            root=Path(scratch);tools=root/'bin';tools.mkdir()
            for name,source in {'ls':'/bin/ls','cat':'/bin/cat','sleep':'/bin/sleep','wc':'/usr/bin/wc','head':'/usr/bin/head'}.items():
                (tools/name).symlink_to(source)
            (root/'sample.txt').write_text('remote fixture\n')
            child,master=pty.fork()
            if child==0:
                os.chdir(root)
                attrs=termios.tcgetattr(0)
                attrs[6][termios.VERASE]=b'\x7f'
                termios.tcsetattr(0,termios.TCSANOW,attrs)
                env={'TERM':'dumb','LANG':'C','LC_ALL':'C','HOME':scratch,'PATH':str(tools),'PS1':'MACBOOK> ','PS2':'MORE> ','HISTFILE':'/dev/null'}
                os.execve('/bin/bash',['bash','--noprofile','--norc','--restricted','--noediting','-i'],env)
            os.set_blocking(master,False)
            to_shell=bytearray();to_serial=bytearray()
            deadline=start+seconds
            record('start',pid=child,term='dumb',queue_limit=LIMIT)
            print(f'READY {device}: MACBOOK> is the remote test shell; local Ctrl+C stops this bridge',flush=True)
            while time.monotonic()<deadline:
                readers=[];writers=[]
                if len(to_shell)<LIMIT:readers.append(fd)
                if len(to_serial)<LIMIT:readers.append(master)
                if to_shell:writers.append(master)
                if to_serial:writers.append(fd)
                readable,writable,_=select.select(readers,writers,[],.05)
                for dest,queue in [(master,to_shell),(fd,to_serial)]:
                    if dest in writable:
                        try:n=os.write(dest,queue[:256])
                        except BlockingIOError:continue
                        if n<=0:raise OSError('zero-length write')
                        record('to-shell' if dest==master else 'to-serial',hex=bytes(queue[:n]).hex());del queue[:n]
                for source,queue in [(fd,to_shell),(master,to_serial)]:
                    if source not in readable:continue
                    try:data=os.read(source,min(256,LIMIT-len(queue)))
                    except BlockingIOError:continue
                    except OSError as error:
                        if source==master and error.errno==errno.EIO:data=b''
                        else:raise
                    if not data:
                        # Drain the final shell output before returning.
                        if source==master:
                            end=time.monotonic()+2
                            while to_serial and time.monotonic()<end:
                                if select.select([],[fd],[],.05)[1]:
                                    n=os.write(fd,to_serial[:256]);record('to-serial',hex=bytes(to_serial[:n]).hex());del to_serial[:n]
                            if to_serial:raise TimeoutError('final output drain timed out')
                        record('eof',side='shell' if source==master else 'serial');return 0
                    queue.extend(data)
            record('timeout');return 124
    finally:
        if child:
            got,status=os.waitpid(child,os.WNOHANG)
            if not got:
                # Closing the controlling PTY signals its foreground job too.
                try:os.kill(child,signal.SIGHUP)
                except ProcessLookupError:pass
            if master is not None:os.close(master)
            end=time.monotonic()+1
            while not got and time.monotonic()<end:
                got,status=os.waitpid(child,os.WNOHANG)
                if not got:time.sleep(.02)
            if not got:
                try:os.kill(child,signal.SIGKILL)
                except ProcessLookupError:pass
                _,status=os.waitpid(child,0)
            record('child-exit',status=status)
        termios.tcsetattr(fd,termios.TCSANOW,original);os.close(fd)
        record('closed')

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('device');p.add_argument('--seconds',type=float,default=600);p.add_argument('--log',type=Path,required=True);a=p.parse_args()
    try:
        with a.log.open('w') as log:return serve(a.device,a.seconds,log)
    except KeyboardInterrupt:return 130
    except (OSError,ValueError) as e:print(f'serial-shell: {e}',file=sys.stderr);return 1
if __name__=='__main__':sys.exit(main())
