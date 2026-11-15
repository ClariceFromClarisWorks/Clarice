#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../src/update/confirmation.h"
int main(void){const char *yes[]={"\n","\r","y\n","Y\n","yes\r\n"," YES \n","\t\r"};const char *no[]={"","n\n","N\n","no\n","NO\r","yes","y","yesxxxxxxxxxxxxxxxxxxxxxx"};for(unsigned i=0;i<sizeof yes/sizeof *yes;i++)assert(update_confirmation(yes[i],strlen(yes[i]))==1);for(unsigned i=0;i<sizeof no/sizeof *no;i++)assert(update_confirmation(no[i],strlen(no[i]))==0);assert(update_confirmation("maybe\n",6)==-1);assert(update_confirmation("y\0\n",3)==-1);puts("PASS: Enter/y/yes, n/no, EOF, truncation, invalid and binary responses");}
