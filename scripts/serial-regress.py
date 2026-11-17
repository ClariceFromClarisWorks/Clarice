#!/usr/bin/env python3
"""Run local remote-shell fixtures; Snow input uses the existing CGEvent helper.
The host shell is NOT Clarice. Only --keys/--snow-pid delivers Clarice input.
Without --snow-pid, tests exercise the host PTY bridge only.
"""
import argparse,json,os,pty,select,signal,subprocess,sys,time,tty
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def run(a):
    a.output.mkdir(parents=True,exist_ok=True)
    wire=a.output/'wire.jsonl';result=a.output/'result.json'
    master=slave=None
    if not a.snow_pid:
        master,slave=pty.openpty();tty.setraw(slave);a.device=os.ttyname(slave)
    if not a.device:raise ValueError('--device required for Snow')
    peer=subprocess.Popen([sys.executable,str(ROOT/'scripts/serial-shell.py'),a.device,'--seconds',str(a.timeout),'--log',str(wire)],stdout=(a.output/'peer.txt').open('w'),stderr=subprocess.STDOUT)
    success=False;results=[];started=time.monotonic();position=0;pending=b''
    def read_wire():
        nonlocal position,pending
        if not wire.exists():return []
        with wire.open('rb') as f:f.seek(position);data=f.read(65536);position=f.tell()
        pending+=data;lines=pending.split(b'\n');pending=lines.pop()
        if len(pending)>8192:raise ValueError('oversized wire record')
        return [json.loads(line) for line in lines]
    def send(data):
        if len(data)>4096:raise ValueError('fixture input exceeds 4096 bytes')
        if master is not None:
            view=memoryview(data)
            while view:
                n=os.write(master,view);view=view[n:]
        else:
            f=a.output/'keys.bin';f.write_bytes(data)
            subprocess.run([str(a.keys),str(a.snow_pid),str(f),str(a.interval)],check=True,timeout=30,stdout=subprocess.DEVNULL)
    try:
        deadline=time.monotonic()+5
        while not wire.exists() or not wire.stat().st_size:
            if peer.poll() is not None:raise RuntimeError('peer failed to start; see peer.txt')
            if time.monotonic()>deadline:raise TimeoutError('peer startup')
            time.sleep(.02)
        for index,step in enumerate(json.loads(a.fixture.read_text())):
            if 'local' in step:
                if a.snow_pid:send(step['local'].encode('ascii'));time.sleep(step.get('wait',1))
                results.append({'step':index,'local_input_submitted':bool(a.snow_pid)});continue
            read_wire() # discard output from preceding step; each expectation is fresh
            data=step['input'].encode('ascii');t=time.monotonic();send(data)
            expected=step.get('expect','').encode('ascii');expected_wire=step.get('wire',step['input']).encode('ascii')
            rx=bytearray();tx=bytearray();deadline=time.monotonic()+step.get('timeout',5)
            while expected not in tx or bytes(rx)!=expected_wire:
                if master is not None and select.select([master],[],[],0)[0]:os.read(master,4096)
                for row in read_wire():
                    if row['event']=='to-shell':rx.extend(bytes.fromhex(row['hex']))
                    if row['event']=='to-serial':tx.extend(bytes.fromhex(row['hex']))
                if len(rx)>8192 or len(tx)>65536:raise ValueError('fixture capture limit exceeded')
                if not expected_wire.startswith(rx):raise AssertionError(f'wire mismatch step {index}: {rx!r}')
                if time.monotonic()>deadline:raise TimeoutError(f'step {index}: rx={rx!r} tx={tx!r}')
                if peer.poll() is not None:raise RuntimeError('peer exited during fixture')
                time.sleep(.01)
            if 'once' in step:assert tx.count(step['once'].encode('ascii'))==1,repr(tx)
            results.append({'step':index,'pass':True,'seconds_including_typing':time.monotonic()-t,'received':rx.hex(),'output':tx.hex()})
        result.write_text(json.dumps({'pass':True,'mode':'Snow keyboard + serial' if a.snow_pid else 'host PTY only','steps':results,'seconds':time.monotonic()-started},indent=2)+'\n')
        success=True;return 0
    except Exception as error:
        result.write_text(json.dumps({'pass':False,'error':str(error),'steps':results},indent=2)+'\n');print(error,file=sys.stderr);return 1
    finally:
        if peer.poll() is None:
            peer.send_signal(signal.SIGINT)
            try:peer.wait(timeout=3)
            except subprocess.TimeoutExpired:peer.kill();peer.wait()
        if master is not None:os.close(master);os.close(slave)
        # A failed Snow test might still own the serial session. Never type a
        # shell command as cleanup; use the existing local cancellation chord.
        if a.snow_pid and not success:
            send(b'\x1c') # helper's explicit Command-period chord, targeted PID
        if result.exists():
            summary=json.loads(result.read_text());summary['peer_returncode']=peer.returncode
            result.write_text(json.dumps(summary,indent=2)+'\n')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--fixture',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--device');p.add_argument('--snow-pid',type=int);p.add_argument('--keys',type=Path,default=ROOT/'work/serial-2/keys');p.add_argument('--interval',type=float,default=.06);p.add_argument('--timeout',type=int,default=120);a=p.parse_args();sys.exit(run(a))
