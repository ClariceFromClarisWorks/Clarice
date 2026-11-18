"""Collect physical-SE counters after local measurements; no timed serial traffic."""
import argparse, json, re
from pathlib import Path
from serial_client import SerialClient

p = argparse.ArgumentParser()
p.add_argument('--password-file', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
c = SerialClient(a.password_file.read_text().strip())
rows = []
try:
    for variant in ('b', 'a'):
        for case in ('buffer0','buffer1','buffer2','buffer3','empty0',
                     'short0','short1','short2','short3','long0'):
            name = '/tmp/n' + variant + case
            status, raw = c.command('cat ' + name + 's ' + name)
            if status != 0:
                raise RuntimeError(raw)
            if not re.search(r'\r?\n0\r?\nRuntime:', raw):
                raise RuntimeError('Editor did not exit successfully: ' + raw)
            marks = list(map(int, re.search(r'Nano ticks: ([0-9 ]+);', raw)[1].split()))
            if len(marks) != 13 or not all(marks[i] for i in range(9)):
                raise RuntimeError('Missing startup markers: ' + raw)
            if 'remaining=0' not in raw:
                raise RuntimeError('Retained allocations: ' + raw)
            row = dict(variant=variant, case=case, marks=marks,
                       ready_ticks=marks[8]-marks[0],
                       ready_seconds=(marks[8]-marks[0])/60,
                       curses_ticks=marks[4]-marks[3],
                       setup_ticks=marks[5]-marks[4],
                       file_ticks=marks[6]-marks[5],
                       layout_ticks=marks[7]-marks[6],
                       refresh_ticks=marks[8]-marks[7], raw=raw)
            for key in ('heap','largest','load','owned-peak','remaining'):
                row[key] = int(re.search(r'\b'+key+r'=(\d+)', raw)[1])
            rows.append(row)
            a.output.write_text(json.dumps(rows, indent=2)+'\n')
            print(variant, case, row['ready_ticks'], row['owned-peak'], flush=True)
    status, raw = c.command('cat /tmp/se1818-pipeout')
    a.output.with_name('pipeline.json').write_text(json.dumps(dict(status=status,text=raw),indent=2)+'\n')
    c.send(b'exit\r')
finally:
    c.close()
