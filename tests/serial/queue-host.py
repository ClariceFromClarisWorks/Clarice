#!/usr/bin/env python3
"""Check actual serial ownership helpers without allocating another idle queue."""
import re,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[2]
s=(root/'src/app.c').read_text()
a=s.index('int app_serial_input(');z=s.index('static void key(',a)
queue=re.search(r'else if\(serial_input\)\{(.*?)\}else if\(c==27',s).group(1)
source='''#include <assert.h>
#include <stdio.h>
#define LINE_CAP 1024
static unsigned char ahead_keys[LINE_CAP];
static unsigned int ahead_head,ahead_count;
static int serial_input,serial_head,serial_count,serial_overflow,blocked,last_cr;
static void input_fail(const char *s){(void)s;blocked=1;}
'''+s[a:z]+'\nstatic void push(unsigned char c){'+queue+'}\n'+'''
int main(void){
 ahead_count=3;ahead_keys[0]='a';ahead_keys[1]='b';ahead_keys[2]='c';
 app_serial_input(0);assert(ahead_count==3); /* ordinary tool cleanup */
 app_serial_input(1);assert(!ahead_count);assert(app_serial_key()=='a');
 app_serial_input(1);assert(app_serial_key()=='b');assert(app_serial_key()=='c');assert(app_serial_key()==-1);
 for(int r=0;r<10;r++){for(int i=0;i<1024;i++)push(i%256);for(int i=0;i<1024;i++)assert(app_serial_key()==i%256);assert(app_serial_key()==-1);}
 for(int i=0;i<1025;i++)push('z');assert(app_serial_key()==-2);assert(app_serial_key()==-2);
 app_serial_input(0);assert(blocked&&!serial_count&&!ahead_count);
 ahead_count=2;app_serial_input(0);assert(ahead_count==2);
 puts("PASS: existing typeahead preserved outside serial, transferred on entry, 10240 ordered queue bytes, fail-closed overflow, no remote suffix replay");
}
'''
d=root/'work/serial-1';(d/'queue-host.c').write_text(source)
subprocess.run(['cc','-fsanitize=address,undefined',str(d/'queue-host.c'),'-o',str(d/'queue-host')],check=True)
subprocess.run([str(d/'queue-host')],check=True)
