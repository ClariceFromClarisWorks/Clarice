# Startup regression / profiling

Run `python3 tests/startup/host.py` (ASan/UBSan actual directory loop, migration
call gate and actual window-position correction). Also run the existing geometry,
namespace volume/rename and terminal viewport/control tests.

`python3 tests/startup/profile.py` makes a disposable source copy under
`work/startup-q1/src`. Build it with the normal Retro68 toolchain into the sibling
`build` directory. It never instruments production source. Use a disposable
Snow Plus/4 MB/System6.0.8 image called Sys6Unix, with Clarice at its root.
Never modify an image with host HFS tools while Snow has it open.

Build this directory's CMake project separately to obtain StartClarice. This
reuses the existing native-benchmark System6 sublaunch convention. It timestamps
`Sys6Unix:launch-q1` and launches Clarice; the diagnostic app appends measurements
to `Sys6Unix:startup-q1` after the first ready draw. Timestamp write/close is included
in native-entry time. This is a matched launch proxy, not Finder mouse latency.
Run once after a guest reboot, then quit/reopen repeatedly at the same fixed
maximum FFWD setting. Host filesystem caches are not flushed: do not call this a
physical cold-storage measurement. Disable diagnostic instrumentation for normal
acceptance and distribution.

Marks relative to C entry: 1 Toolbox, 2 menus, 23 first window frame, 3 positioned
window/early feedback, 4 controls/ports, 13 shell start, 17 namespace path,
18 directory checks, 14 namespace complete, 15 legacy reports, 16 history open,
10 shell ready, 5 preference path, 6 preferences restored, 7 font/layout,
8 greeting buffered, 19 update-marker check complete, 20 profile preflight,
21 profile read, 22 profile execution complete, 9 profile return,
11 ready state, 12 ready screen drawn. `paint` includes startup terminal draws.
An old build without Starting text has no visible message at mark3; its first
terminal text is painted during profile execution (marks21–22).

Normal-build checks: saved font/theme/geometry, typing before Return, Ctrl+C130,
scrolling and editor exit; repeat launches for heap stability. In a disposable
image move MacTCP/DNR/Prep out of System Folder and reboot to test absence.
Preserve the existing Clarice Data folder under another name before a fresh-Core
startup check; restore it afterwards. Keep any legacy migration fixtures and
compare their contents, including a destination collision that must not overwrite.
