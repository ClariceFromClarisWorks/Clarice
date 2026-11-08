from pathlib import Path
root=Path(__file__).resolve().parents[1]
resources=[]
for name,resource_id in (("core",200),("grep",201),("sed",202)):
 b=(root/f'work/build/{name}.clx').read_bytes()
 resources.append("data 'CLX1' (%d) {\n"%resource_id+'\n'.join(' $"'+b[i:i+32].hex()+'"' for i in range(0,len(b),32))+'\n};\n')
(root/'work/build/Commands.r').write_text(''.join(resources))
