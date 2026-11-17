# PowerBook control connection — 17 November 2026

User identifies PowerBook 180. Installed Clarice 2026111550 reports 68030,
68882, 10 MB RAM and 640×400 at 4-bit depth. This is not acceptance of the
experimental renderer or an independent verification of every hardware field.

Immediate return to Finder after “Connected” was resolved by assigning a separate
host listener to the PowerBook’s trusted LAN address. The previous listeners
accepted only the SE. No guest helper or OS modification was required.

Verified: sustained connection, 640×400 monochrome screen capture, keyboard input
in the helper, mouse switch to Finder, background capture, launching the installed
Clarice from System Folder, intact 12-character fast input and successful pwd / ls
/bin with prompt recovery. Burst delivery took 0.217 seconds host time; screenshot
capture took 30–45 guest ticks plus roughly 1.2–3.8 seconds host transfer time.
These are control measurements, not stock-SE rendering comparisons.

The installed application is intentionally beside Clarice Data in System Folder,
as implemented by the offline installer. No builds, user data or recovery copies
were moved or deleted. SEControl’s separate intermittent timeout remains open;
its orderly disconnect exit must not be conflated with a proven memory crash.
