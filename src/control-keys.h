#ifndef CLARICE_CONTROL_KEYS_H
#define CLARICE_CONTROL_KEYS_H
/* EventRecord modifier bits. Menus win for Command alone; Command+Option
   explicitly requests Control in Command mode. Physical Control always works. */
static inline const char *control_name(int mode){return mode==0?"control":mode==2?"command":"option";}
static inline int control_parse(const char *s){return !strcmp(s,"control")?0:!strcmp(s,"option")?1:!strcmp(s,"command")?2:-1;}
static inline int control_requested(int mode,unsigned mods){return (mods&0x1000)||(mode==1&&(mods&0x0800))||(mode==2&&(mods&0x0100));}
static inline unsigned char control_ascii(unsigned char c){if(c==' '||c=='@')return 0;if(c=='?')return 127;if((c>='a'&&c<='z')||(c>='A'&&c<='_'))return c&31;return c;}
#endif
