"""Generate MacBinary fixtures with deliberately different birth/modify dates.
Install these into an isolated HFS test folder; +i a separate empty folder too.
"""
from pathlib import Path
import datetime,struct,binascii
base=Path('work/modules');base.mkdir(parents=True,exist_ok=True);epoch=datetime.datetime(1904,1,1)
for name,flags in [('visible',0),('invisible',0x4000),('.dot',0)]:
 h=bytearray(128);h[1]=len(name);h[2:2+len(name)]=name.encode();h[65:73]=b'TEXTTEST';h[73]=flags>>8
 birth=int((datetime.datetime(2000,2,29,12,34)-epoch).total_seconds());modified=int((datetime.datetime(2025,1,1)-epoch).total_seconds())
 struct.pack_into('>IIII',h,83,4,0,birth,modified);h[102:106]=b'mBIN';h[122]=h[123]=129;h[124:126]=binascii.crc_hqx(h[:124],0).to_bytes(2,'big')
 (base/(name+'.bin')).write_bytes(h+b'test'+bytes(124))
