"""Create disposable inputs; no real volumes are modified by this generator."""
from pathlib import Path
import sys
w=Path(sys.argv[1]);w.mkdir(parents=True,exist_ok=True)
for name,data in {'small':b'z\na\nb\na\n','long':b'z'*20000+b'\n'+b'a'*20001+b'\n'+b'z'*20000+b'\n','record':b'x'*65536+b'\n','oom':b'x'*600000+b'\n','fields':b'x '*2000+b'end\n','read127':b'a'*127+b'\nnext\n','read128':b'a'*128+b'\nnext\n','readlong':b'a'*8192+b'\nnext\n'}.items():(w/name).write_bytes(data)
