/* Test-only dispatch selection: use the real native Serial Driver while a
 * localhost Telnet harness supplies commands/cancellation and collects results.
 * The production transfer engine, file staging and native transport are intact. */
static long native_only(int op,void *p,unsigned long n){(void)op;(void)p;(void)n;return 0;}
#define cli_remote_wire native_only
#include "../../src/commands/kermit.c"
