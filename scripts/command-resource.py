from pathlib import Path
root=Path(__file__).resolve().parents[1]
resources=[]
for name,resource_id in (("core",200),("grep",201),("sed",202),("awk",203),("serial",204),("wc",205),("head",206),("cut",207),("tr",208),("uniq",209),("tee",210),("tail",211),("sort",212),("files",213),("inspect",214),("misc",215),("basename",216),("dirname",217)):
 b=(root/f'work/build/{name}.clx').read_bytes()
 resources.append("data 'CLX1' (%d) {\n"%resource_id+'\n'.join(' $"'+b[i:i+32].hex()+'"' for i in range(0,len(b),32))+'\n};\n')
(root/'work/build/Commands.r').write_text(''.join(resources))
