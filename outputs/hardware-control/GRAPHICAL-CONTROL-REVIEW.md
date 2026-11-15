# SEControl graphical control — input architecture boundary

14 November 2026. Investigation stopped before implementing a new input backend,
under the task's explicit requirement to identify unsupported/invasive foreground
input techniques before proceeding. Existing physical screenshot/key proof is
preserved. No new helper build, Snow run or physical test is claimed here.

## What can stay on the existing path

The helper already declares canBackground and acceptSuspendResumeEvents in SIZE
and calls WaitNextEvent. A persistent host session, background screenshots and
paced key events are reasonable incremental work, though background operation
still needs actual testing. Apple's MultiFinder note explicitly describes one
event queue delivering background-posted events to the foreground application.
Thus foreground keyboard routing does not itself require an OS patch.
[Apple TB35](https://leopard-adc.pepas.com/technotes/tb/tb_35.html).

The earlier physical result remains two Wi-Fi captures in 1.198/1.086 seconds and
one visually confirmed x delivered to the frontmost helper. It did not prove
background input, Finder interaction, mouse control or Clarice lifecycle.

## Concrete blocker to the complete requested interface

Posting a mouseDown/mouseUp with a chosen event position is not a complete mouse
device. PPostEvent lets a caller change the queued event fields; it does not
provide a supported continuous pointer-movement command. GetMouse obtains current
pointer location independently of the queued event's where field.
[PPostEvent](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-66.html),
[GetMouse](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-72.html).

This is a real consumer requirement in the existing Clarice source, not just a
hypothetical VNC feature: `src/app.c`'s selection and scrollbar tracking loops
call GetMouse/StillDown/SystemTask; title dragging calls DragWindow. These paths
do not call WaitNextEvent/GetNextEvent/EventAvail while tracking. MultiFinder
schedules ordinary application contexts at those event calls, so a helper that
only polls TCP in its main loop cannot be assumed to receive/process further
movement and release messages during the drag. A queued down/up pair also does
not provide the successive positions used by GetMouse.
[Apple switching rules](https://leopard-adc.pepas.com/technotes/tb/tb_35.html),
[DragWindow](https://dev.os9.ca/techpubs/mac/Toolbox/Toolbox-245.html).

Inspected MiniVNC commit 5941457e1fca5f676dadea0008f64225a4543a27:

- VNCServer.cpp lines 99–114 directly write low-memory cursor/button state.
- vncPointerEvent, lines 843–889, sets mouse positions, cursor update state,
  button state and MBTicks. Its explicit Plus workaround keeps MBTicks ahead
  of the clock to stop the VBL handler overwriting synthetic button state.
- vncProcessMessages, around lines 777–783, dispatches input in MacTCP completion
  context (`asInterrupt`), rather than depending exclusively on its main loop.

[Pinned source](https://github.com/marciot/mac-minivnc/blob/5941457e1fca5f676dadea0008f64225a4543a27/mac-cpp-source/VNCServer.cpp).

Some of those locations have LowMem accessor names; that does not establish a
supported System 6 remote-input contract for the combined technique, especially
the timing workaround. These are transient state writes, **not ROM modifications
or trap replacement**. Nevertheless they manipulate system-global input state
and require interrupt-safe scheduling/lifetime handling. The present synchronous
MacTCP helper cannot simply run its allocating/parsing/UI routines in a completion
callback. A stuck synthetic button or a callback into freed state is a concrete
risk if implemented casually.

This establishes a gap in the existing event-only design and a known broader
implementation path. It does not prove that every possible System 6 solution is
impossible. No unsupported writes, patches or input experiments were performed
on the physical SE during this investigation.

## Practical alternatives requiring a scope decision

1. **Approve a narrowly bounded transient mouse-state backend.** Retain no ROM
   or trap patches. First prove in disposable Snow a fixed-size prevalidated
   input sequence with a suitable interrupt-time mechanism, proper context
   preservation and explicit button release/cleanup. No allocation, UI drawing,
   filesystem access or synchronous TCP from that callback. Qualify SE ADB
   behavior separately from the Plus-specific MBTicks workaround; do not copy
   it blindly. Then add persistent host controls and physical Finder/Clarice
   validation. This is the recommended software-only path if these state writes
   fit the owner's definition of permitted development infrastructure. It is
   not implemented, and no success or exact memory cost is promised.
2. **Keep the strict event-only boundary.** Build background capture/keyboard
   controls and qualify simple queued clicks individually, with human mouse
   movement/drag/focus assistance where needed. This can be useful but does not
   meet the full graphical-control acceptance criteria.
3. **Use an external ADB keyboard/mouse emulator.** This can avoid software
   pointer-state injection on the SE, while SEControl supplies screenshots.
   Requires additional verified hardware; the existing USB serial cable is not
   an ADB injector. No hardware purchase or implementation assumed.

No new graphical interface or installation package is delivered at this boundary;
the existing proof remains available. No new performance measurements. The
previous low heap headroom (4,432 bytes with snapshot) must also be addressed or
qualified before adding callbacks/buffers; do not claim it is safe by inference.

Deployment scope is corrected for future work: SEControl is graphical development
infrastructure only. No comprehensive file transfer or updater here. Future
Clarice wget, appinstall and clarice update are the intended deployment path;
manual deployment is acceptable now. Earlier report proposals for an SEControl
network updater are superseded by this task's direction.

Clarice production code, System 6, ROM and BlueSCSI firmware remain unchanged.
No listener was started, no push/publication, and no unattended recovery work.
