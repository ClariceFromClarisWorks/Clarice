"""Actual viewport + new admission gate; model/pixel reuse with caret sentinels."""
from pathlib import Path
import subprocess
r=Path(__file__).resolve().parents[2];s=(r/'src/app.c').read_text();v=(r/'tests/terminal/viewport-host.py').read_text();head=v.split("code=r'''",1)[1].split("'''+s[a:b]",1)[0]
a=s.index('static void input_position');b=s.index('static void pstr',a)
gate=s[s.index('static int input_wrap_scroll'):s.index('static void draw_native(void){')]
code=head+'static char last_screen[40][COLS];static long drawn_first;static int drawn_prompt;\n'+s[a:b]+gate+r'''
int main(void){char next[40][COLS],pixels[40][COLS];long first;int prompt,count=0;
 for(columns=8;columns<=100;columns+=2)for(visible=3;visible<=37;visible+=2)for(int lines=1;lines<=7;lines++){
  memset(text,0,sizeof text);current_line=120;oldest_line=0;for(int i=0;i<120;i++)for(int x=0;x<columns;x++)text[i][x]='!'+(i+x)%90;
  int n=columns*lines-5;if(n<0||n>1022)continue;memset(input,'x',n);input[n]=0;cursor=n;
  compose_input(last_screen,&drawn_first,&drawn_prompt);memcpy(pixels,last_screen,sizeof pixels);
  input[n]='z';input[n+1]=0;cursor++;compose_input(next,&first,&prompt);
  assert(input_wrap_scroll(next,first,prompt));
  /* The production move excludes the bottom row. Copying its models alongside
     the pixels preserves the exact patch oracle, including cursor metadata. */
  memmove(pixels,pixels+1,(visible-1)*COLS);memmove(last_screen,last_screen+1,(visible-1)*COLS);memset(last_screen+visible-1,255,COLS);
  for(int row=0;row<=visible;row++)if(memcmp(last_screen[row],next[row],COLS))memcpy(pixels[row],next[row],COLS);
  assert(!memcmp(pixels,next,(visible+1)*COLS));count++;
  scrollback=1;assert(!input_wrap_scroll(next,first,prompt));scrollback=0;
  assert(!input_wrap_scroll(next,first+1,prompt));next[visible][COLS-1]=0;assert(!input_wrap_scroll(next,first,prompt));
 }
 printf("PASS %d wrap transitions, model/pixel pairing, multirow inputs, caret and admission boundaries\n",count);
}
'''
p=r/'work/terminal-performance/wrap-host.c';p.write_text(code)
subprocess.run(['cc','-fsanitize=address,undefined',str(p),'-o',str(p.with_suffix(''))],check=True);subprocess.run([str(p.with_suffix(''))],check=True)
