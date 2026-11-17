/* Host stand-in for the same invocation allocator used by the actual adapter. */
#include <stdlib.h>
void *cli_alloc(unsigned long n) { return malloc(n); }
void cli_free(void *p) { free(p); }
