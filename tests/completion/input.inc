/* Deterministic guest Event Manager input during a nonmatching catalog scan.
   Uses production Tab/key/poll/replay/draw paths; absent from normal builds. */
int app_completion_probe(unsigned long out[5],int mode){
 int cancel=mode==1;Ptr reserve=0;
 const char *original="cat /tmp/completion/n1000/z";
 busy=0;cancelled=interrupted=0;ahead_head=ahead_count=0;strcpy(input,original);cursor=strlen(input);dirty=1;draw();
 if(mode==2){long n=MaxBlock()-32768;if(n>0)reserve=NewPtr(n);}
 completion_inject=cancel?3:'X';completion_polls=completion_posted=0;completion_delay=0;out[2]=FreeMem();
 key(9,0);int good=cancel?(!*input&&shell_status==130):(!strcmp(input,original)&&ahead_count==1);
 while(ahead_count){unsigned at=ahead_head;ahead_head=(ahead_head+1)%LINE_CAP;ahead_count--;key(ahead_keys[at],ahead_mods[at]);}
 if(!cancel)good=good&&!strcmp(input,"cat /tmp/completion/n1000/zX");
 dirty=1;draw();out[0]=completion_delay;out[1]=TickCount()-completion_post_tick;out[3]=FreeMem();out[4]=MaxBlock();if(reserve)DisposePtr(reserve);
 good=good&&completion_posted&&!dirty;completion_inject=0;input[0]=0;cursor=0;cancelled=interrupted=0;busy=1;return good;
}
