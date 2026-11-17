#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "../../src/control-keys.h"
int main(void){for(int mode=0;mode<3;mode++){assert(control_parse(control_name(mode))==mode);assert(control_requested(mode,0x1000));assert(!control_requested(mode,0));assert(!!control_requested(mode,0x800)==(mode==1));assert(!!control_requested(mode,0x100)==(mode==2));}assert(control_parse("invalid")==-1);for(int c='a';c<='z';c++)assert(control_ascii(c)==(c&31));for(int c='A';c<='_';c++)assert(control_ascii(c)==(c&31));assert(control_ascii(' ')==0&&control_ascii('?')==127);puts("PASS: all modifier modes, ASCII control letters/punctuation, physical Control always available");}
