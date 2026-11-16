#ifndef TEST_PARSER_FAULT_H
#define TEST_PARSER_FAULT_H
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
extern size_t pf_calls,pf_live,pf_peak,pf_fail;
extern int pf_hit,pf_sticky;
void *pf_malloc(size_t);void *pf_calloc(size_t,size_t);void *pf_realloc(void *,size_t);void pf_free(void *);char *pf_strdup(const char *);char *pf_strndup(const char *,size_t);
#ifndef PF_IMPLEMENTATION
#define malloc pf_malloc
#define calloc pf_calloc
#define realloc pf_realloc
#define free pf_free
#define strdup pf_strdup
#define strndup pf_strndup
#endif
#endif
