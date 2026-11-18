from pathlib import Path
import sys
root=Path(sys.argv[1]);root.mkdir(parents=True,exist_ok=True)
(root/'nshort').write_bytes((b'a'*63+b'\n')*128)
(root/'nlong').write_bytes((b'a'*4095+b'\n')*2)
(root/'n64').write_bytes(b'a'*65535+b'\n')
(root/'n16').write_bytes(b'a'*8191+b'\0'+b'b'*8191+b'\n')
