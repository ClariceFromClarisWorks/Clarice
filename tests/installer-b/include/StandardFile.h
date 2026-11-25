typedef struct {short v,h;} Point;
typedef struct {int good;short vRefNum;} SFReply;
static int picker_good;
static void SFPutFile(Point p,const unsigned char*a,const unsigned char*b,void*c,SFReply*r){(void)p;(void)a;(void)b;(void)c;r->good=picker_good;r->vRefNum=1;}
