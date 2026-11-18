"""Assert USB CDC control lines while C-Kermit owns the open serial port.
No reads or writes; opening this descriptor does not consume peer data.
"""
import os,fcntl,termios,struct,sys,re
assert len(sys.argv)==2 and re.fullmatch(r'/dev/cu\.usbmodem[0-9]+',sys.argv[1])
fd=os.open(sys.argv[1],os.O_RDWR|os.O_NOCTTY|os.O_NONBLOCK)
try:
 a=termios.tcgetattr(fd);a[2]|=termios.CLOCAL|termios.CREAD;termios.tcsetattr(fd,termios.TCSANOW,a)
 fcntl.ioctl(fd,termios.TIOCMBIS,struct.pack('I',termios.TIOCM_DTR|termios.TIOCM_RTS))
finally:os.close(fd)
