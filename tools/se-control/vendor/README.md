# Reused network source

`nettcp.c` and `nettcp.h`: Bart Decrem's Macinclaude, MIT licence in LICENSE.
Upstream commit `f4b06160b186c95f0ebc234214fe916ae2b463b9`, files `net/nettcp.c`
and `net/nettcp.h`: https://github.com/bdecrem/Macinclaude

Local changes: use the pinned toolchain's Multiversal MacTCP declarations rather
than duplicate Apple headers; represent StreamPtr as Ptr; cast Pascal driver
name; initialize parameter blocks; reduce open/send timeout from 20 to 5 seconds;
return negative OSErr correctly; never dispose a system receive ring if
TCPRelease failed. No networking changes to Clarice or the operating system.

The capture approach (snapshot qd.screenBits) was informed by Macinclaude's
`wifi/screen.inc`. This proof sends raw packed monochrome bytes over TCP instead
of its serial-modem hex/PackBits/multiplexer protocol. Mouse-state handling in ../mouse.c is adapted from MiniVNC (Marcio Teixeira),
commit 5941457e1fca5f676dadea0008f64225a4543a27, VNCServer.cpp
vncPointerEvent: MouseTemp/RawMouseLocation/CursorNew and Plus MBTicks handling.
The standalone helper and its new mouse/timer integration are GPL-3.0-or-later;
see LICENSE-MiniVNC.txt. Macinclaude files retain their MIT notices.
No AppleBridge code or VNC protocol is incorporated.

Additional adapter change: after draining unread bytes, TCPStatus detects a
connection state other than ESTABLISHED (8), so a peer FIN does not leave the
helper polling for five minutes. See Apple MacTCP Programmer's Guide, TCPStatus.
