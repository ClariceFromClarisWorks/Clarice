/* Experimental, discardable Monaco 9 history rows. No terminal state lives
   here. A purged/failed allocation is an ordinary cache miss. Keep this handle
   locked only between begin/end; never retain a pointer across that boundary.
   80 slots leave room within the 100 KiB experimental allowance for a separate
   24 KiB glyph table. Pixel presentation still uses the caller's clipped path. */
#define HISTORY_CACHE_SLOTS 80
#define HISTORY_CACHE_KEY_BYTES 132
#define HISTORY_CACHE_ROW_BYTES (64*11)
typedef struct {
 long line;
 unsigned short valid; /* Keep key and pixel block even-aligned for BlockMove. */
 unsigned char key[HISTORY_CACHE_KEY_BYTES];
 unsigned char pixels[HISTORY_CACHE_ROW_BYTES];
} HistoryRasterRow;
static Handle history_cache_handle;
static HistoryRasterRow *history_cache_rows;
static int history_cache_columns,history_cache_phase;
static short history_cache_font,history_cache_size;
static unsigned long history_cache_retry;
static int history_cache_waiting;

static void history_cache_dispose(void){
 history_cache_rows=0;
 if(history_cache_handle){DisposeHandle(history_cache_handle);history_cache_handle=0;}
 history_cache_waiting=0;
}
static void history_cache_end(void){
 history_cache_rows=0;
 if(history_cache_handle){HUnlock(history_cache_handle);HPurge(history_cache_handle);}
}
static int history_cache_begin(int columns,int phase,short font,short size,int cell,int height){
 int fresh=0;
 if(history_cache_rows)return 0; /* No nested ownership. */
 if(font!=4||size!=9||cell!=6||height!=11||columns<1||columns>85||phase<0||phase>15||phase+columns*6>512)return 0;
 if(history_cache_handle&&!*history_cache_handle){DisposeHandle(history_cache_handle);history_cache_handle=0;}
 if(!history_cache_handle){
  unsigned long now=TickCount();
  if(history_cache_waiting&&(unsigned long)(now-history_cache_retry)<60)return 0;
  history_cache_handle=NewHandle((Size)(sizeof(HistoryRasterRow)*HISTORY_CACHE_SLOTS));
  if(!history_cache_handle){history_cache_retry=now;history_cache_waiting=1;return 0;}
  history_cache_waiting=0;fresh=1;
 }
 HNoPurge(history_cache_handle);HLock(history_cache_handle);
 history_cache_rows=(HistoryRasterRow*)*history_cache_handle;
 if(!history_cache_rows){history_cache_end();return 0;}
 if(fresh||columns!=history_cache_columns||phase!=history_cache_phase||font!=history_cache_font||size!=history_cache_size){
  for(int i=0;i<HISTORY_CACHE_SLOTS;i++)history_cache_rows[i].valid=0;
  history_cache_columns=columns;history_cache_phase=phase;history_cache_font=font;history_cache_size=size;
 }
 return 1;
}
static HistoryRasterRow *history_cache_slot(long line){
 if(!history_cache_rows||line<0)return 0;
 return &history_cache_rows[(unsigned long)line%HISTORY_CACHE_SLOTS];
}
static int history_cache_load(long line,const unsigned char *key,unsigned char *pixels){
 HistoryRasterRow *r=history_cache_slot(line);
 if(!r||!r->valid||r->line!=line||memcmp(r->key,key,HISTORY_CACHE_KEY_BYTES))return 0;
 BlockMove(r->pixels,pixels,HISTORY_CACHE_ROW_BYTES);return 1;
}
static void history_cache_store(long line,const unsigned char *key,const unsigned char *pixels){
 HistoryRasterRow *r=history_cache_slot(line);if(!r)return;
 r->valid=0;BlockMove(key,r->key,HISTORY_CACHE_KEY_BYTES);BlockMove(pixels,r->pixels,HISTORY_CACHE_ROW_BYTES);r->line=line;r->valid=1;
}
