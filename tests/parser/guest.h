#ifndef CLARICE_PARSER_GUEST_FAULT_H
#define CLARICE_PARSER_GUEST_FAULT_H
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
void *parser_test_malloc(size_t);
void *parser_test_calloc(size_t,size_t);
void *parser_test_realloc(void *,size_t);
void parser_test_free(void *);
char *parser_test_strdup(const char *);
char *parser_test_strndup(const char *,size_t);
#define malloc parser_test_malloc
#define calloc parser_test_calloc
#define realloc parser_test_realloc
#define free parser_test_free
#define strdup parser_test_strdup
#define strndup parser_test_strndup
#endif
