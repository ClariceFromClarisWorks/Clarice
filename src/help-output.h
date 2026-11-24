#ifndef CLARICE_HELP_OUTPUT_H
#define CLARICE_HELP_OUTPUT_H

#include <string.h>

/* Each source line is a paragraph. Indented lines hang by two spaces.
   A tab separates an option label from its description. */
static int help_format(int width, int fd, const char *text,
                       long (*write_chunk)(int, const void *, unsigned long),
                       int (*poll)(void))
{
 char buffer[264]; /* Two maximum-width terminal lines, including newlines. */
 unsigned used=0;
 if(width<=0)width=66;
 if(width>131)width=131;
 while(*text){
  const char *end=text,*tab=0;
  while(*end&&*end!='\n'){
   if(*end=='\t'&&!tab)tab=end;
   ++end;
  }
  const char *label=text;
  while(text<end&&*text==' ')++text;
  int indent=(int)(text-label);
  if(indent>=width)indent=0;
  int hang=indent&&indent+2<width?indent+2:indent;
  const char *part_end=tab?tab:end;
  int aligned=0;
  if(tab&&width>=48&&tab-label<=14){
   const char *word=tab+1;
   while(word<end&&*word!=' ')++word;
   if(word-(tab+1)<=width-16){
    aligned=(int)(tab-label);
    indent=hang=16;
    text=tab+1;tab=0;part_end=end;
   }
  }
  do{
   int room=width-indent;
   int count=part_end-text>room?room:(int)(part_end-text);
   if(text+count<part_end&&text[count]!=' '){
    int split=count;
    while(split&&text[split-1]!=' ')--split;
    if(split)count=split-1;
    /* Preserve a word that fits only without its hanging indent. */
    else if(indent&&!aligned){
     int word=count;
     while(word<width&&text+word<part_end&&text[word]!=' ')++word;
     if(text+word==part_end||text[word]==' '){count=word;indent=0;}
    }
   }
   while(count&&text[count-1]==' ')--count;
   if(used+width+1>sizeof buffer){
    if(write_chunk(fd,buffer,used)!=(long)used)return 1;
    if(poll&&poll())return 130;
    used=0;
   }
   if(count||aligned){
    memcpy(buffer+used,label,aligned);used+=aligned;
    memset(buffer+used,' ',indent-aligned);used+=indent-aligned;
    memcpy(buffer+used,text,count);used+=count;
    aligned=0;
   }
   buffer[used++]='\n';
   text+=count;
   while(text<part_end&&*text==' ')++text;
   indent=hang;
   if(text==part_end&&tab){
    text=tab+1;tab=0;part_end=end;
    indent=hang=4<width?4:0;
   }
  }while(text<end);
  text=*end?end+1:end;
 }
 if(used&&write_chunk(fd,buffer,used)!=(long)used)return 1;
 return poll&&poll()?130:0;
}

#ifdef __m68k__
extern int app_columns(void),app_yield(void);
extern long cli_write(int,const void *,unsigned long);
static inline int help_output(int fd,const char *text)
{
 return help_format(app_columns(),fd,text,cli_write,app_yield);
}
#else
/* Prepared ports also build as host executables for regression tests. */
#include <stdio.h>
static long help_host_write(int fd,const void *data,unsigned long size)
{
 return fwrite(data,1,size,fd==2?stderr:stdout);
}
static inline int help_output(int fd,const char *text)
{
 return help_format(66,fd,text,help_host_write,0);
}
#endif

#endif
