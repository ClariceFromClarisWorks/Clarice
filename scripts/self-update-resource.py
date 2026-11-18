"""Embed the updater with the same Rez data encoding as command-resource.py."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
b=(root/'work/update-command/clarice.clx').read_bytes()
assert b[:4]==b'CLX1'
(root/'work/update-command/SelfUpdateData.r').write_text("data 'CLXU' (128, purgeable) {\n"+'\n'.join(' $"'+b[i:i+32].hex()+'"' for i in range(0,len(b),32))+'\n};\n')
