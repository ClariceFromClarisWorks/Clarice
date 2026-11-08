#!/usr/bin/env python3
"""Manage only this project's disposable Snow instance; never edit original media."""
from pathlib import Path
import subprocess,os,signal,sys,time
ROOT=Path(__file__).resolve().parents[1]; RUN=ROOT/'work/emulator'
IMAGE='ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c'
def running():
    if not (RUN/'pid').exists(): return False
    pid=int((RUN/'pid').read_text())
    p=subprocess.run(['ps','-p',str(pid),'-o','command='],capture_output=True,text=True)
    return p.returncode==0 and str(RUN/'Sys6Snow.app/Contents/MacOS/Snow') in p.stdout
mode=sys.argv[1]
if mode=='stop':
    if running():
        os.kill(int((RUN/'pid').read_text()),signal.SIGTERM)
        for _ in range(40):
            if not running():break
            time.sleep(.1)
    if running():raise SystemExit('Owned emulator did not stop')
elif mode=='install':
    if running():raise SystemExit('Stop the owned emulator before updating its disk')
    subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(ROOT)+':/src',IMAGE,'sh','-ec','hmount /src/work/emulator/system.hda; hcopy -m /src/work/build/Clarice.bin :Clarice; humount'],check=True)
elif mode in ('start','start608'):
    if running():raise SystemExit('Already running')
    workspace=ROOT/'work/system608/plus.snoww' if mode=='start608' else RUN/'plus.snoww'
    if not workspace.exists():raise SystemExit('Test workspace is not prepared: '+str(workspace))
    log=(workspace.parent/'snow.log').open('w')
    p=subprocess.Popen([str(RUN/'Sys6Snow.app/Contents/MacOS/Snow'),str(workspace)],stdout=log,stderr=log,start_new_session=True)
    (RUN/'pid').write_text(str(p.pid));print('Started',p.pid)
else:raise SystemExit('Use start, start608, stop or install (install targets the original project test disk)')
