#ifndef CLARICE_IOCTL_H
#define CLARICE_IOCTL_H
#define TIOCGWINSZ 0x5413
#define FIONREAD 0x541b
struct winsize {unsigned short ws_row,ws_col,ws_xpixel,ws_ypixel;};
int ioctl(int,unsigned long,...);
#endif
