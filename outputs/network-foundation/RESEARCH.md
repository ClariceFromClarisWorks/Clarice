# Native networking foundation — interface decisions

17 November2026. Independent branch/worktree `codex/native-network-foundation`
at `Clarice-Network`, based on0686839. The accepted0.5 checkout, running SE,
network configuration and distribution are not modified.

## Sources inspected before implementation

- Apple, *MacTCP Programmer's Guide*, chapters5/6 (DNR,GetMyIPAddr,ICMP).
  Existing reference: [PDF mirror](https://github.com/marciot/mac-minivnc/blob/master/docs/MacTCP_programming.pdf).
- [MoofTCP](https://github.com/matthewmrichter/mooftcp),
  a72e90ae8dd65e99a7be4981f15da85e1795e1c7: mac/glue/mt_pb_ip.c,
  mac/dnr/mt_dnr.c, mac/apiprobe/apiprobe_dnr.inc, mac/include/mt_mactcp.h,
  and mac/heapinfo/dnrheap.c. MIT; Matthew Richter. Study/reference only;
  no IP stack or private MoofTCP control codes copied into Clarice.
- [MoofPort](https://github.com/matthewmrichter/moofport),
  18a0383461d8b3a4aaa75d20e8528078c40e4a76: docs/dev/INTERFACE.md,
  docs/dev/RESEARCH.md, installer/source and README. MIT; Matthew Richter.
- Existing attributed Macinclaude MIT TCP adapter retained. BetterTelnet DNR
  callback/lifetime approach inspected as historical reference; GPL source not
  transplanted. Public ABI structures independently declared where the pinned
  Multiversal headers lack them, with target compile-time offset assertions.

## Smallest boundary

Command-local shared object/source provides address/mask, system DNR resolution
and native ICMP echo. Existing TCP adapter remains the transport. No resident
service, sockets, private DNS packets, routing table, Ethernet protocol handler,
new loader import or permanent shell allocation. Linker discards unused services.

`.IPP` csCode15 supplies local IPv4/subnet mask. Do not infer link health, gateway,
DNS-server list, DHCP mode, stack brand/version or CPU speed from those values.
DNR supplies name-to-address translation including the stack's cache, default
suffix and configured servers. `dnrp` is loaded on demand and closed/released.
`.ENET0` belongs below the installed TCP/IP stack. Public EGetInfo can supply a
MAC address for an Ethernet driver, but opening/probing a presumed Ethernet
interface is unnecessary for this foundation and would not describe every
MacTCP transport. No MoofPort tuning/private selectors are called.

Gateway, server/suffix selection, DHCP/static mode and Ethernet configuration
remain owned by the installed control panel/driver. ipconfig is read-only and
reports only the public address/mask facts. Neither brand of stack nor Ethernet
driver is selected or required by Clarice's code.

## Callback ownership and limits

DNR immediate results and cacheFault/deferred completion are different paths.
Apple explicitly says CloseResolver must wait until all outstanding calls finish.
MoofTCP's stronger private cancellation is therefore NOT used. A deadline or
Ctrl+C records failure/cancellation, pumps events and drains the outstanding
native callback before closing/freeing. A stuck installed resolver can prolong
this drain; this is NOT a hard wall-clock cancellation guarantee.

ICMP submission completion is NOT echo completion. The echo callback receives
the driver's own block in A0; its userData points to invocation-owned state.
The callback does no allocation/Toolbox/UI work. Submission PB, payload and
callback code remain alive until both submission and reply/timeout complete.
Ctrl+C drains the outstanding echo (requested reply timeout2s by default).
There is no documented public ICMP abort operation here. A broken stack that
never calls back cannot safely be escaped by unloading this CLX image; do not
add a watchdog that frees live callback addresses. A hard-abort requirement would
need a separately reviewed lifetime mechanism, not a false success/unsafe free.

Host mocks verify ordering and ownership, not real stack interoperability.
Disposable Snow driver installations are test fixtures only. No physical stack
or control-panel settings are changed; System7.1 remains untested.

## Open Transport scope — 17 November 2026

Apple's [Open Transport1.1 requirements FAQ](https://til-2001.mirror.kb1max.com/techinfo.nsf/artnum/n19437/)
(source: Apple Tech Info Library19437, mirrored) requires68030/68040 or supported
PowerPC, System7.1/7.1.1/7.1.2 or supported later releases, and at least5MB on68k.
It is not a System6/68000/4MB SE option. A suitably configured PowerBook180 is
a potential future validation target. Apple explicitly documents backward
compatibility with the MacTCP programming interface. First test the existing
`.IPP` TCP, resolver-resource discovery and ICMP paths under that compatibility
layer; do not assume all three work because the general API is supported. Only
add a native OT adapter for a demonstrated gap/benefit. No OT implementation,
installation or compatibility acceptance is included in this milestone.
