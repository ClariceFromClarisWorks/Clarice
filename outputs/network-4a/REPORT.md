# Batch 4A — serial lifetime and FTP completion

Accepted build2026111971, product30d7b1e on local main; baseline 2026111970/3fc4251. Normal Retro68 `-m68000`
build; disposable Snow Plus/4 MiB/System 6.0.8 fixture. No physical access.

- Serial/Kermit previously returned after KillIO without confirming completion;
  failed buffer restoration/driver closes likewise allowed CLX storage to expire.
  Shared foreground cleanup now drains the asynchronous PB, restores the receive
  buffer and closes each driver before releasing invocation storage. Each failure
  is reported once; failed operations retain ownership and retry with a one-tick
  native delay. Repeated successful cleanup is harmless. No new retained state.
- NetAvailable/NetRecv previously collapsed native errors into apparent closure.
  They now preserve OSErr and distinguish drained CLOSE_WAIT/connectionClosing
  from failure. FTP rejects errors before committing staged downloads, including
  unknown SIZE with a final226. Telnet and wget preserve native failure status.
  Queued bytes are consumed before orderly EOF; no extra status calls are added.

## Acceptance

- Baseline negative controls reproduce unsafe Serial/Kermit lifetime and false
  FTP success. Corrected ASan/UBSan tests cover failed/delayed KillIO, pending PB
  and data ownership, timeout, failed buffer restoration and close, retry and
  repeated cleanup. Native Kermit fixture:20 cycles; TCP system-buffer fixture:
  20 cycles, failed release retains storage and subsequent release frees it.
- FTP, HTTP, libtelnet/VT, shared network and remote-service host regressions pass.
  Existing Kermit protocol fixtures pass binary/empty, corruption retry, cancel
  and recovery. Run `sh tests/network-4a/run-host.sh` for affected host checks.
- Snow normal FTP: binary upload/download with exact byte comparison, empty and
  unknown-size orderly EOF, ten repeated transfers, Telnet IP/raw Ctrl+C130,
  prompt recovery and no staged residue. Test-only native-error injection after
  real received bytes fails with -23012 despite final226; no target is installed.
- Snow production Serial: text shell, escape/disconnect/reconnect. Native pending
  request probe:three cancellations, pending=1, KillIO=0, completion=-27, rc130,
  heap delta0 each. Native Kermit probe selects the Serial Driver rather than
  Telnet raw-wire transport: binary roundtrip, cancellation130, retry and empty
  transfer pass. Its production engine/native adapter are otherwise unchanged.
- Shipped Kermit local binary receive/byte comparison and Ctrl+C130 pass;
  shipped FTP local Command-period130 preserves prompt and cleans staging.
- Normal-build local keyboard checks use 1x Snow to avoid accelerated injected
  key-repeat artefacts; other functional tests use fixed maximum acceleration.
  Test scripts/evidence explicitly distinguish probes from shipped commands.
- Offline HFS readback compares Core data/resource forks and type/creator, Release
  and all49 utilities. Offline tar entries match the same sealed build exactly.

## Cost and limits

FTP repeated-run free/largest application heap:324926/323090 bytes before and
after; system heap:45622/45508 before and after. Kermit native probe after each
of five transfers:324526/323090 application,45622/45508 system. Runtime remaining
command allocations0. Heap measurements exclude the disposable probe's own image
when comparing different commands; repeated samples use the same probe/state.
No new persistent allocation, buffer or startup work. wget has4 extra invocation
BSS bytes. CLX package deltas:serial+476, Kermit+424, wget+84, FTP+196, Telnet+100
(total1280bytes); Core MacBinary size unchanged438784bytes. No speedup claimed.

A permanently broken Serial Driver that never completes/detaches keeps foreground
cleanup waiting. Returning to the prompt/unloading its memory would be unsafe;
this fix deliberately does not invent a resident orphan-I/O service.
Snow's NAT bridge delivered the forced host reset as orderly EOF; native errors
were therefore also tested by deterministic guest/host injection. Real stack
reset behaviour and physical timing remain hardware acceptance requirements.
The established CLOSE_WAIT14 interpretation is corroborated by pinned MoofTCP
`mac/glue/mt_pb_tcp.c` (a72e90ae8dd65e99a7be4981f15da85e1795e1c7,
status and closing_check); no upstream code copied.

## Publication

Canonical HTTP/AFP verified:51 manifest entries,36 unique payloads, matching
sealed source/build, offline tar and HDA. Prior1970 index, payloads and recovery
material retained. See verification.json; no GitHub push or physical deployment.
