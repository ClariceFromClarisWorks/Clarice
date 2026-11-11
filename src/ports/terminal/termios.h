/* Clarice virtual terminal modes, not a serial-driver termios implementation. */
#ifndef CLARICE_TERMIOS_H
#define CLARICE_TERMIOS_H
#include <sys/types.h>
typedef unsigned long tcflag_t;
typedef unsigned long speed_t;
typedef unsigned char cc_t;
#define NCCS 16
struct termios {tcflag_t c_iflag,c_oflag,c_cflag,c_lflag;cc_t c_cc[NCCS];speed_t c_ispeed,c_ospeed;};
#define VEOF 0
#define VEOL 1
#define VERASE 2
#define VINTR 3
#define VKILL 4
#define VMIN 5
#define VQUIT 6
#define VSTART 7
#define VSTOP 8
#define VSUSP 9
#define VTIME 10
#define ICRNL 1
#define INLCR 2
#define IGNCR 4
#define IXON 8
#define IXOFF 16
#define BRKINT 32
#define PARMRK 64
#define INPCK 128
#define ISTRIP 256
#define IGNBRK 512
#define OPOST 1
#define ONLCR 2
#define TABDLY 4
#define TAB3 4
#define ICANON 1
#define ISIG 2
#define IEXTEN 4
#define ECHO 8
#define ECHOE 16
#define ECHOK 32
#define ECHONL 64
#define NOFLSH 128
#define TOSTOP 256
#define CSIZE 3
#define CS8 3
#define PARENB 4
#define CLOCAL 8
#define CREAD 16
#define TCSANOW 0
#define TCSADRAIN 1
#define TCSAFLUSH 2
#define TCIFLUSH 0
#define TCOFLUSH 1
#define TCIOFLUSH 2
#define B0 0
#define B50 50
#define B75 75
#define B110 110
#define B134 134
#define B150 150
#define B200 200
#define B300 300
#define B600 600
#define B1200 1200
#define B1800 1800
#define B2400 2400
#define B4800 4800
#define B9600 9600
#define B19200 19200
#define B38400 38400
int tcgetattr(int,struct termios *);
int tcsetattr(int,int,const struct termios *);
int tcflush(int,int);
speed_t cfgetospeed(const struct termios *);
speed_t cfgetispeed(const struct termios *);
int cfsetospeed(struct termios *,speed_t);
int cfsetispeed(struct termios *,speed_t);
#endif
