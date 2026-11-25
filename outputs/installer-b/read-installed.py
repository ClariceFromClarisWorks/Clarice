from pathlib import Path
import sys,subprocess,shlex,json,struct,binascii,re
root=Path.cwd();p=root/'work/installer-b';image,out,base,core=sys.argv[1:];dest=p/out;dest.mkdir(exist_ok=True)
pairs=re.findall(r'CLARICE_COMMAND\("([^"]+)", "([^"]+)"\)',(root/'src/standard-commands.inc').read_text())
commands=['hmount /src/work/installer-b/'+shlex.quote(image)]
for native,name,mode in [(core,'Core.bin','m'),(base+'etc:clarice:installed.catalog','catalog','r'),(':System Folder:Clarice Locations','Locations','r')]+[(base+'bin:'+n,n,'r')for n,_ in pairs]:
 commands.append('hcopy -'+mode+' '+shlex.quote(native)+' /src/work/installer-b/'+shlex.quote(out+'/'+name))
commands+=['hls -la '+shlex.quote(base+'etc:clarice:'),'humount'];s=p/(out+'.sh');s.write_text('set -eu\n'+'\n'.join(commands)+'\n')
r=subprocess.run(['docker','run','--rm','--pull','never','--platform','linux/amd64','--network','none','-v',str(root)+':/src','ghcr.io/autc04/retro68@sha256:dd923969100f408943f73c9618b3c7628686aae819583eecb0ddd1837da6943c','sh','/src/work/installer-b/'+s.name],stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True);(p/(out+'.txt')).write_text(r.stdout);assert r.returncode==0,r.stdout
for n,img in pairs:assert(dest/n).read_bytes()==(root/f'work/build/{img}.clx').read_bytes(),n
rows={}
for line in(dest/'catalog').read_text().splitlines()[1:]:
 k,n,v,l,c=line.split('\t');rows[k,n]=int(v),int(l),int(c,16)
assert rows['core','Clarice'][0]==2026111977
for n,_ in pairs:
 b=(dest/n).read_bytes();assert rows['tool',n]==(2026111977,len(b),binascii.crc32(b)),n

def forks(b):
 d,r=struct.unpack_from('>II',b,83);off=128+((d+127)//128)*128;return b[65:73],b[128:128+d],b[off:off+r]
a,b=forks((dest/'Core.bin').read_bytes()),forks((root/'work/build/Clarice.bin').read_bytes());assert a[:2]==b[:2];assert a[2][:16]+a[2][128:]==b[2][:16]+b[2][128:]
l=(dest/'Locations').read_bytes();assert l[:8]==b'CLLOC001';assert struct.unpack_from('>I',l,520)[0]==binascii.crc32(l[:520]);print('PASS',out,'Core+49 utilities equal sealed kit, full catalog and location record; meaningful Core forks/Finder codes verified')
