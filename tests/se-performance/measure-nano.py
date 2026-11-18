"""Bounded local nano runs through existing SEControl, with per-run screenshots.
Never saves or modifies fixtures. Stats are collected separately over serial after
remote services are re-enabled. Check every screenshot and return status before
accepting measurements. First in session is not cold boot.
"""
import argparse,json,subprocess,time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--socket',required=True);p.add_argument('--variant',choices=['b','a'],required=True);p.add_argument('--cases',default='buffer0,buffer1,buffer2,buffer3,empty0,short0,short1,short2,short3,long0');a=p.parse_args()
root=Path(__file__).resolve().parents[2];out=root/'work/se-performance';log=out/('nano-actions-'+a.variant+'.json');rows=json.loads(log.read_text()) if log.exists() else []
def control(*args):
 start=time.monotonic();r=subprocess.run(['python3',str(root/'tools/se-control/control.py'),'--socket',a.socket,*args],capture_output=True,text=True,timeout=130)
 try:data=json.loads(r.stdout)
 except ValueError:raise RuntimeError(r.stdout+r.stderr)
 if r.returncode or not data.get('ok'):raise RuntimeError(data)
 rows.append(dict(action=list(args),start_monotonic=start,end_monotonic=time.monotonic(),result=data))
 (out/('nano-actions-'+a.variant+'.json')).write_text(json.dumps(rows,indent=2)+'\n')
 return data
for case in a.cases.split(','):
 kind=case[:-1];assert kind in ['buffer','empty','short','long'] and case[-1].isdigit()
 path={'buffer':'','empty':' /tmp/ne18','short':' /tmp/n8s18','long':' /tmp/n8l18'}[kind]
 control('type','nano-profile'+path,'--enter')
 time.sleep(13)
 control('shot',str(out/('nano-'+a.variant+'-'+case+'.png')))
 # Non-modifying cursor key verifies that the editor can service input.
 control('key','right');time.sleep(.5);control('key','ctrl+x');time.sleep(1)
 control('type','echo $? > /tmp/n'+a.variant+case+'s; status runtime > /tmp/n'+a.variant+case,'--enter')
 time.sleep(1)
 print('captured',a.variant,case,flush=True)
