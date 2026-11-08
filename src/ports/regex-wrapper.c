#include "compat.h"
#include <regex.h>
/* A failed match allocation is an error, never an ordinary non-match. */
int port_regexec(const regex_t *re,const char *s,size_t n,regmatch_t *matches,int flags){
 int rc=regexec(re,s,n,matches,flags);
 if(rc&&rc!=REG_NOMATCH){fprintf(stderr,"regex execution failed (code %d)\n",rc);port_exit(2);}
 return rc;
}
