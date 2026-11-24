from pathlib import Path
import shutil
r=Path(__file__).resolve().parents[2];w=r/'work/word-navigation';s=w/'src'
if s.exists():shutil.rmtree(s)
shutil.copytree(r/'src',s)
if not (w/'work').exists():(w/'work').symlink_to('..',target_is_directory=True)
p=s/'app.c';t=p.read_text().replace('int main(void){','#include "/src/tests/word-navigation/guest.inc"\nint main(void){').replace(' while(!done){',' while(!done){static int tested;if(!tested&&FrontWindow()==win){tested=1;word_probe();}',1);p.write_text(t)
