#ifndef CLARICE_MEMORY_INFO_H
#define CLARICE_MEMORY_INFO_H
#include <stdint.h>
/* Runtime 1.4: current application zone only, no machine-wide free RAM claim. */
typedef struct {uint32_t heap_extent,free_bytes,largest,after_purge,largest_after_purge;} ClariceMemory;
int cli_memory(ClariceMemory *,unsigned long);
#endif
