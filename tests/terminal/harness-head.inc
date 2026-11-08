#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#define ROWS 128
#define COLS 132
static char text[ROWS][COLS];
static int test_ref=-1,columns,visible=22,retained,out_col,scrollback,last_cr,dirty;
static long current_line,oldest_line;
static void FSWrite(int ref,long *n,const char *s){(void)ref;(void)n;(void)s;abort();}
static int scroll_limit(void){long newest=current_line-visible+1,oldest=oldest_line;if(newest<0)newest=0;if(oldest<0)oldest=0;return newest>oldest?(int)(newest-oldest):0;}
static void newline(void){current_line++;if(current_line-retained+1>oldest_line)oldest_line=current_line-retained+1;out_col=0;memset(text[current_line%ROWS],0,COLS);if(scrollback){scrollback++;if(scrollback>scroll_limit())scrollback=scroll_limit();}}

static unsigned char wrapped[ROWS];
static void soft_newline(void){wrapped[current_line%ROWS]=1;newline();}

static unsigned long activity_output_tick;
static void activity_hide(void){}
static unsigned long TickCount(void){return 0;}
