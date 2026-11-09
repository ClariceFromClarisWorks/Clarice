"""Create disposable HFS guest fixture inputs under work/, never beside /."""
from pathlib import Path
root=Path(__file__).resolve().parents[2]
out=root/'work/pipelines-fixtures';out.mkdir(parents=True,exist_ok=True)
(out/'pipeline-data').write_bytes(b'alpha one\nbeta two\nbeta two\ngamma three\n')
(out/'pipeline-long').write_bytes(b'x'*17000+b'\n')
(out/'pipeline-large').write_bytes(b'alpha one beta two gamma three\n'*20000)
