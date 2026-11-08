#ifndef CLARICE_REFLOW_H
#define CLARICE_REFLOW_H
#define TERM_ROWS 128
#define TERM_COLS 132
typedef struct {char lines[TERM_ROWS][TERM_COLS];unsigned char wraps[TERM_ROWS];long end,oldest,anchor;int column;} TerminalReflow;
void terminal_reflow(TerminalReflow *,const char [TERM_ROWS][TERM_COLS],const unsigned char *,long,long,long,int,int);
#endif
