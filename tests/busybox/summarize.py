import json,re,statistics
from pathlib import Path
rows=json.loads(Path('outputs/busybox/measurements.json').read_text());summary=[]
for editor in ('bbvi','vi','xvi'):
 for hasfile in (False,True):
  rr=[r for r in rows if r['editor']==editor and ('/tmp/' in r['command'])==hasfile]
  if not rr:continue
  def values(k):return [round(r[k],4) for r in rr]
  stats=[{k:int(v) for k,v in re.findall(r'([\w-]+)=(\d+)',r['metrics'].split('\r\n')[1])} for r in rr]
  summary.append(dict(editor=editor,input='8KiB' if hasfile else 'empty',ready_seconds=values('host_submit_to_full_screen_seconds'),responsive_seconds=values('host_submit_to_key_response_seconds'),whole_seconds=values('host_complete_open_key_quit_seconds'),key_seconds=values('host_key_response_seconds'),startup_bytes=[r['startup_bytes'] for r in rr],runtime=stats))
Path('outputs/busybox/summary.json').write_text(json.dumps(summary,indent=2)+'\n')
for s in summary:print(s['editor'],s['input'],s['ready_seconds'],s['whole_seconds'],[(r.get('owned-peak'),r.get('ticks'),r.get('load'),r.get('heap')) for r in s['runtime']])
