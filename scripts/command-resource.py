from pathlib import Path
root=Path(__file__).resolve().parents[1]
b=(root/'work/build/core.clx').read_bytes()
(root/'work/build/Commands.r').write_text("data 'CLX1' (200) {\n"+'\n'.join(' $"'+b[i:i+32].hex()+'"' for i in range(0,len(b),32))+'\n};\n')
