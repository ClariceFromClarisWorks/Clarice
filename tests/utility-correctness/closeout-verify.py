from pathlib import Path
import re,json
r=Path(__file__).resolve().parents[2];p=r/'outputs/utility-closeout'
s=(p/'final.out').read_text();assert s.startswith('LS:0\nDF:0\nBAD_LS:2\n');assert s.count('FIND:0\n')==3
assert s.endswith('CLEANUP:0\nOPERANDS_DONE\n')
assert (p/'final.err').read_text()=='usage: ls [-lah1P] [--full] [--] [path ...]\n'
assert len((p/'many-df').read_text().splitlines())==101
assert sum(x=='/tmp/operands/file' for x in (p/'many-ls').read_text().splitlines())==100
assert not (p/'bad-ls').read_bytes()
assert (p/'found').read_text()=='/tmp/operands/file\n'
assert (p/'closeout.out').read_text().startswith('PROFILE_STATUS:130\n')
assert (p/'cancel.out').read_text().startswith('CANCEL:130\n/mnt/Audit/tools\n')
rows=[tuple(map(int,m)) for m in re.findall(r'Runtime: heap=(\d+) largest=(\d+).*?remaining=(\d+)',s)]
assert len(rows)==3 and all(x[2]==0 for x in rows)
assert max(x[0] for x in rows)-min(x[0] for x in rows)<=2
assert len(set(x[1] for x in rows))==1
(p/'guest-summary.json').write_text(json.dumps(dict(build=2026111546,guest='Snow Plus / 4 MB / reported System 6.0.7',physical_acceptance=False,ls_operands=100,df_operands=100,profile_cancel=130,prompt_cancel=130,repeated_find_heap=rows),indent=2)+'\n')
print('PASS: 100 ls/df operands, empty invalid-option output, repeated find heap/cleanup, startup and prompt cancellation')
