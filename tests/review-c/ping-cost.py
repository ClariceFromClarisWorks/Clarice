"""Match command scheduling at identical synthetic guest-clock increments."""
from pathlib import Path
import tempfile,subprocess
r=Path(__file__).resolve().parents[2];s=(r/'tests/network/commands.c').read_text();s=s[:s.index('int main(void){')]+'''int main(void){char*p[]={"ping","-c","2","-W","1","example.test"};assert(!run_ping(6,p));printf("two probes: elapsed=%lu ticks busy_polls=%d pause_calls=%d\\n",now,polls,pauses);assert(calls==2&&now==60);}\n'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)
 for label in ('before','after'):
  code=subprocess.check_output(['git','show','59e5116:src/commands/ping.c'],cwd=r,text=True) if label=='before' else (r/'src/commands/ping.c').read_text()
  (p/'ping.c').write_text(code)
  test=s.replace('"../../src/help-output.h"','"'+str(r/'src/help-output.h')+'"').replace('"../../src/commands/ping.c"','"'+str(p/'ping.c')+'"').replace('"../../src/commands/ipconfig.c"','"'+str(r/'src/commands/ipconfig.c')+'"')
  (p/'test.c').write_text(test);subprocess.run(['cc','-I'+str(r/'src/commands'),'-I'+str(r/'tests/network/include'),str(p/'test.c'),'-o',str(p/'test')],check=True)
  print(label,subprocess.check_output([str(p/'test')],text=True).strip())
