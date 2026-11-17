from pathlib import Path
import re,json,statistics,sys
p=Path(sys.argv[1]);text=p.read_text();rows=[]
for part in text.split('CASE ')[1:]:
 name,repeat=part.splitlines()[0].split();status=re.search(r'STATUS (\d+)',part);runtime=re.search(r'Runtime: (.*)',part)
 row={'case':name,'repeat':int(repeat),'status':int(status[1]) if status else None}
 if runtime:row.update({k:int(v) for k,v in re.findall(r'([\w-]+)=(\d+)',runtime[1])})
 for label in ['Loader ticks','Verification ticks']:
  m=re.search(label+r': (.*)',part)
  if m:row[label]={k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',m[1])}
 m=re.search(r'real ([\d.]+) s',part)
 if m:row['shell_seconds']=float(m[1])
 rows.append(row)
Path(str(p)+'.json').write_text(json.dumps({'complete':'AUDIT_DONE' in text,'rows':rows},indent=2)+'\n')
for name in dict.fromkeys(x['case'] for x in rows):
 a=[x for x in rows if x['case']==name];print(name,[(x.get('ticks'),x.get('load'),x['status'],x.get('owned-peak'),x.get('remaining')) for x in a])
print('Complete:', 'AUDIT_DONE' in text)
