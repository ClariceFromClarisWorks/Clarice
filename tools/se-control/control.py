#!/usr/bin/env python3
"""SEControl persistent session and local command API. No HTTP or file transfer.
serve listens for the SE; other subcommands use an owner-only Unix socket.
"""
import argparse
import hmac
import json
import os
from pathlib import Path
import select
import socket
import sys
import time
from host import Wire, capture, local_address

LETTERS = dict(zip('abcdefghijklmnopqrstuvwxyz', [0,11,8,2,14,3,5,4,34,38,40,37,46,45,31,35,12,15,1,17,32,9,13,7,16,6]))
DIGITS = dict(zip('0123456789', [29,18,19,20,21,23,22,26,28,25]))
PUNCT = dict(zip(" `-=[]\\;',./", [49,50,27,24,33,30,42,41,39,43,47,44]))
SHIFT = dict(zip('~_+{}|:"<>?!@#$%^&*()', "`-=[]\\;',./1234567890"))
NAMED = {'return':(13,36), 'enter':(13,36), 'tab':(9,48), 'escape':(27,53),
         'backspace':(8,51), 'left':(28,123), 'right':(29,124), 'up':(30,126), 'down':(31,125)}


def character(c):
    if c in '\r\n':return 13,36,0
    if c == '\t':return 9,48,0
    if c.isascii() and c.lower() in LETTERS:
        return ord(c),LETTERS[c.lower()],512 if c.isupper() else 0
    if c in DIGITS:return ord(c),DIGITS[c],0
    if c in PUNCT:return ord(c),PUNCT[c],0
    if c in SHIFT:
        base=SHIFT[c];return ord(c),(PUNCT|DIGITS)[base],512
    raise ValueError('Unsupported character; this proof uses US ASCII keyboard layout')


def key_spec(spec):
    parts=spec.lower().split('+');mods=0
    for p in parts[:-1]:
        if p not in ('cmd','shift','option','ctrl'):raise ValueError('Unknown modifier')
        mods |= {'cmd':256,'shift':512,'option':2048,'ctrl':4096}[p]
    last=parts[-1]
    if last in NAMED:ch,code=NAMED[last];return ch,code,mods
    if len(last)!=1:raise ValueError('Unknown key')
    ch,code,auto=character(last.upper() if mods&512 and last.isalpha() else last)
    if mods&4096 and 'a'<=last<='z':ch=ord(last)-96
    return ch,code,mods|auto


def point(x,y):
    if type(x)!=int or type(y)!=int or not 0<=x<512 or not 0<=y<342:
        raise ValueError('Coordinates must be inside 512x342 screen')


def response(w,command,expected=None):
    w.send(command);reply=w.line()
    if reply.startswith('ERR') or (expected and reply!=expected):raise RuntimeError(reply)
    return reply


def status(w):
    line=response(w,'PING');parts=line.split()
    if len(parts)!=8 or parts[0]!='PONG':raise ValueError('Invalid PONG: '+line)
    result={'ticks':int(parts[1]),'free_heap':int(parts[2])}
    for item in parts[3:]:
        k,v=item.split('=');result[k]=int(v)
    return result


def execute(w,request):
    op=request['op']
    if op=='status':return status(w)
    if op=='shot':return capture(w,Path(request['path']))
    if op=='quit':return {'reply':response(w,'QUIT','BYE')}
    if op=='cancel':response(w,'CANCEL','CANCELLING')
    elif op in ('key','type'):
        if op=='key':keys=[key_spec(request['key'])]
        else:
            text=request['text']
            if not isinstance(text,str) or len(text)>1024:raise ValueError('Maximum 1024 characters per request')
            keys=[character(c) for c in text] # validate ALL before sending any
        fast=bool(request.get('fast',False))
        if fast and (op!='type' or any(c in text for c in '\r\n')):
            raise ValueError('Fast typing accepts one unsubmitted line; inspect it before sending Return')
        sent=0
        if fast:
            # Four events fit comfortably within the classic event queue. The
            # installed helper already accepts multiple newline-framed commands.
            for start in range(0,len(keys),4):
                group=keys[start:start+4]
                w.send('\n'.join('KEY %d %d %d'%key for key in group))
                failure=None
                for _ in group:
                    reply=w.line()
                    if reply=='QUEUED':sent+=1
                    elif failure is None:failure=reply
                if failure:raise RuntimeError('Fast typing stopped after %d acknowledged keys; inspect partial line: %s'%(sent,failure))
                time.sleep(.10)
            return {'queued':sent,'delivery':'unsubmitted line; verify screenshot before Return','batch':4}
        try:
            for key in keys:
                response(w,'KEY %d %d %d'%key,'QUEUED');sent+=1;time.sleep(.10)
        except (OSError,ValueError,RuntimeError) as e:
            raise RuntimeError('Stopped after %d/%d queued keys; do not retry blindly: %s'%(sent,len(keys),e)) from e
        return {'queued':sent,'delivery':'verify screenshot; not an application acknowledgement'}
    elif op in ('move','click','drag'):
        x,y=request['x'],request['y'];point(x,y)
        if op=='move':cmd='MOVE %d %d'%(x,y)
        elif op=='click':
            count=request.get('count',1)
            if type(count)!=int or count not in (1,2):raise ValueError('Click count must be 1 or 2')
            cmd='CLICK %d %d %d'%(x,y,count)
        else:
            x2,y2=request['x2'],request['y2'];point(x2,y2);ticks=request.get('ticks',60)
            if type(ticks)!=int or not 30<=ticks<=180:raise ValueError('Drag duration must be 30..180 ticks')
            cmd='DRAG %d %d %d %d %d'%(x,y,x2,y2,ticks)
        response(w,cmd,'QUEUED')
        if request.get('nowait'):return {'queued':True}
    else:raise ValueError('Unknown operation')
    deadline=time.monotonic()+10
    while time.monotonic()<deadline:
        s=status(w)
        if not s['active']:
            if s['down'] or s['error']:raise RuntimeError('Mouse release/event error: '+str(s))
            return s
        time.sleep(.05)
    raise TimeoutError('Mouse completion deadline; action has independent guest release bound')


def read_request(conn):
    conn.settimeout(3);b=bytearray()
    while len(b)<16384:
        c=conn.recv(1)
        if not c:raise ConnectionError('Local request disconnected')
        if c==b'\n':return json.loads(b)
        b.extend(c)
    raise ValueError('Oversized local request')


def serve(args):
    token=args.token_file.read_text().strip()
    if len(token)!=32 or any(c not in '0123456789abcdef' for c in token):raise ValueError('Invalid pairing token')
    args.socket.parent.mkdir(parents=True,exist_ok=True)
    if args.socket.exists():raise FileExistsError('Control socket exists; stop old server/remove stale socket explicitly')
    wire=None;sock=None;heartbeat=0;quiet_until=0
    with socket.socket() as listener,socket.socket(socket.AF_UNIX) as local:
        listener.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
        listener.bind((args.bind,args.port));listener.listen(1)
        oldmask=os.umask(0o077)
        try:local.bind(str(args.socket))
        finally:os.umask(oldmask)
        local.listen(2)
        print(json.dumps({'state':'listening','bind':args.bind,'port':args.port}),flush=True)
        try:
            while True:
                ready,_,_=select.select([listener,local],[],[],1)
                if listener in ready:
                    new,peer=listener.accept()
                    if peer[0]!=args.peer or wire:new.close()
                    else:
                        try:
                            candidate=Wire(new,5)
                            if not hmac.compare_digest(candidate.line(),'HIL2 '+token):raise PermissionError('Pairing/version mismatch')
                            wire=Wire(new,getattr(args,"response_timeout",20));sock=new;heartbeat=time.monotonic();quiet_until=0
                            print(json.dumps({'state':'connected','peer':peer[0]}),flush=True)
                        except (OSError,ValueError) as e:
                            new.close();print(json.dumps({'state':'rejected','error':str(e)}),flush=True)
                if local in ready:
                    client,_=local.accept()
                    with client:
                        try:
                            req=read_request(client);op=req.get('op')
                            if op=='stop':
                                if wire:
                                    response(wire,'CANCEL');response(wire,'QUIT','BYE')
                                client.sendall(b'{"ok":true,"stopped":true}\n');break
                            if op=='quiet':
                                seconds=req.get('seconds')
                                if type(seconds)!=int or not 0<=seconds<=180:raise ValueError('Quiet interval must be 0..180 seconds')
                                if not wire:raise RuntimeError('SE not connected')
                                quiet_until=time.monotonic()+seconds
                                result={'quiet_seconds':seconds,'note':'Automatic heartbeats paused; explicit requests still send traffic'}
                            elif op=='disconnect':
                                if sock:sock.close()
                                sock=wire=None;result={'disconnected':True}
                            else:
                                if not wire:raise RuntimeError('SE not connected')
                                result=execute(wire,req);heartbeat=time.monotonic()
                                if op=='quit':sock.close();sock=wire=None
                            client.sendall((json.dumps({'ok':True,**result})+'\n').encode())
                        except (OSError,ValueError,RuntimeError,KeyError,TypeError) as e:
                            try:client.sendall((json.dumps({'ok':False,'error':str(e)})+'\n').encode())
                            except OSError:pass
                            # A transport error invalidates framing. Never replay input.
                            if isinstance(e,OSError) and sock:sock.close();sock=wire=None
                if wire and time.monotonic()>=quiet_until and time.monotonic()-heartbeat>15:
                    try:status(wire);heartbeat=time.monotonic()
                    except (OSError,ValueError,RuntimeError) as e:
                        print(json.dumps({'state':'disconnected','error':str(e)}),flush=True)
                        sock.close();sock=wire=None
        finally:
            if sock:sock.close()
            args.socket.unlink(missing_ok=True)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--socket',type=Path,default=Path(__file__).resolve().parents[2]/'work/se-control/control.sock')
    sub=p.add_subparsers(dest='op',required=True)
    s=sub.add_parser('serve');s.add_argument('--bind',type=local_address,required=True);s.add_argument('--peer',type=local_address,required=True)
    s.add_argument('--port',type=int,default=5968);s.add_argument('--token-file',type=Path,required=True)
    s.add_argument('--response-timeout',type=int,choices=range(1,121),default=20,metavar='1..120',help='Bounded response wait; use 120 for foreground hardware benchmarks')
    s=sub.add_parser('quiet');s.add_argument('seconds',type=int,choices=range(181),metavar='0..180',help='Pause automatic heartbeats during measurements; below guest idle timeout')
    for name in ('status','cancel','quit','disconnect','stop'):sub.add_parser(name)
    s=sub.add_parser('shot');s.add_argument('path',type=lambda x:str(Path(x).resolve()))
    s=sub.add_parser('key');s.add_argument('key')
    s=sub.add_parser('type');s.add_argument('text');s.add_argument('--enter',action='store_true');s.add_argument('--fast',action='store_true',help='Batch four keys; single line only, no --enter')
    for name in ('move','click','drag'):
        s=sub.add_parser(name);s.add_argument('x',type=int);s.add_argument('y',type=int);s.add_argument('--nowait',action='store_true')
        if name=='click':s.add_argument('--count',type=int,default=1)
        if name=='drag':s.add_argument('x2',type=int);s.add_argument('y2',type=int);s.add_argument('--ticks',type=int,default=60)
    args=p.parse_args()
    if args.op=='serve':serve(args);return
    req=vars(args).copy();req.pop('socket')
    if req.pop('enter',False):req['text']+='\r'
    with socket.socket(socket.AF_UNIX) as client:
        client.settimeout(180);client.connect(str(args.socket));client.sendall((json.dumps(req)+'\n').encode());b=bytearray()
        while len(b)<16384:
            c=client.recv(1)
            if not c:raise ConnectionError('Local server closed response')
            if c==b'\n':break
            b.extend(c)
        answer=json.loads(b);print(json.dumps(answer))
        if not answer.get('ok'):raise SystemExit(1)


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError) as e:
        print(json.dumps({'ok':False,'error':str(e)}));sys.exit(1)
