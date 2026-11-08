import ctypes,datetime,random,subprocess
from pathlib import Path
subprocess.run(['cc','-shared','-fPIC','-O2','src/commands/created.c','-o','work/modules/created.dylib'],check=True)
lib=ctypes.CDLL(str(Path('work/modules/created.dylib').resolve()));lib.format_created.argtypes=[ctypes.c_char_p,ctypes.c_ulong]
epoch=datetime.datetime(1904,1,1);rng=random.Random(68000)
values=[0,1,59,60,86399,86400,2**32-1]+[rng.randrange(2**32) for _ in range(10000)]
# Every day boundary in the complete unsigned HFS timestamp range.
values += list(range(0,2**32,86400))
for seconds in values:
 b=ctypes.create_string_buffer(17);lib.format_created(b,seconds)
 assert b.value.decode()==(epoch+datetime.timedelta(seconds=seconds)).strftime('%Y-%m-%d %H:%M'),(seconds,b.value)
print(f'{len(values)} creation-date cases match Python datetime, including every HFS-range day boundary')
