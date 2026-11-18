from pathlib import Path
import re,json,statistics
p=Path('outputs/ls-codes');s=(p/'results.txt').read_text();m=list(re.finditer(r'(?m)^(file|screen):(\d+):(\d+):(before|after)\n',s));rows=[]
for i,x in enumerate(m):
 b=s[x.end():m[i+1].start() if i+1<len(m) else len(s)];assert re.search(r'(?m)^status:0$',b)
 t=re.search(r'real ([0-9.]+) s',b);r=re.search(r'Runtime: heap=(\d+) largest=(\d+) image=(\d+) ticks=(\d+) load=(\d+) owned-peak=(\d+) remaining=(\d+)',b);assert t and r and r[7]=='0'
 rows.append(dict(mode=x[1],size=int(x[2]),run=int(x[3]),variant=x[4],seconds=float(t[1]),heap=int(r[1]),largest=int(r[2]),image=int(r[3]),ticks=int(r[4]),load=int(r[5]),remaining=int(r[7])))
assert len(rows)==24
assert re.findall(r'(?m)^unchanged:(\d+)$',s)==['0','0']
for n,c in [('legend','0'),('full','0'),('invalid','2')]:assert re.findall(r'(?m)^'+n+r':(\d+)$',s)==[c]
for n in [100,1000]:
 old=(p/f'out-{n}-before.txt').read_bytes();new=(p/f'out-{n}-after.txt').read_bytes();lines=new.splitlines(keepends=True)
 assert len(lines)==n+2 and b''.join(line[:26]+line[36:] for line in lines)==old
 assert len(new)-len(old)==10*(n+2)
 print(n,'entries + dots:',len(new)-len(old),'extra output bytes; other columns identical')
legend=(p/'legend.txt').read_text();full=(p/'full.txt').read_text()
for content in [legend,full]:
 assert 'SITx SITx' in content and 'CLX1 CLRC' in content and '---- ----' in content
 assert 'TYPE/CREA:' in content and 'CREATED:' in content
 assert 'TYPE CREA CREATED          NAME\n' in content
assert 'FLAGS HEX ' in full and 'FLAGS HEX ' not in legend
assert not (p/'bad.txt').read_bytes() and 'usage: ls' in (p/'error.txt').read_text()
for mode,n in [('file',100),('file',1000),('screen',5)]:
 result={}
 for v in ['before','after']:
  a=[x['seconds'] for x in rows if x['mode']==mode and x['size']==n and x['variant']==v]
  assert len(a)==4;result[v]=dict(first=a[0],repeats=a[1:],median=statistics.median(a[1:]))
 b=result['before']['median'];a=result['after']['median'];print(mode,n,json.dumps(result),'delta',round(a-b,3),'s',round((a/b-1)*100,2),'%')
(p/'measurements.json').write_text(json.dumps(rows,indent=2)+'\n')
print('PASS 24 timed commands; exact +10 byte long-format extension; plain/full unchanged; actual HFS type/creator metadata, legends, invalid options and cleanup')
