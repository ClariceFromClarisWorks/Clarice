# Batch 4E — Seriald startup ownership

Normal68000 build2026111975, local main; baseline1974/f9b5f73.
Host and disposable Snow Plus/4MiB/System6.0.8 only. No hardware or GitHub push.

Seriald returned after initialization/registration failure even when native
cleanup failed. Its unregistered CLX image then lost the system-storage owner.
It now shares Telnetd's startup-failure exit, retaining ownership until stop
succeeds and yielding one event tick between failed serial cleanup attempts.
Repeated stop after success now accepts an absent driver. No lifecycle redesign.

## Acceptance

- Existing-source negative control fails the new zero-owner assertion.
- Actual Seriald ASan/UBSan fixture:60 startup failures (20 each: output opening,
  reset, registration), repeated detach/input-close/output-close failures,
  cancellation reported during retry, eventual safe disposal, idempotent stop,
  allocation failure and subsequent successful login/async-write cleanup pass.
- Existing remote service, session, authentication, input-wait, TCP ownership,
  HTTP/FTP/Telnet/VT regression runners pass (network4B/4C). No new assertions
  weaken the previously accepted coverage.
- Snow native probe calls the actual Serial Driver, injecting failures before
  native operations.15 cases on1974 and15 again on1975: every application and
  system heap delta0, retry counts3/5/9, no remaining native owner. The probe
  is separately labelled and excluded from distribution.
- Normal optional Seriald: three complete recorded start/login/command/read
  Ctrl+C130/recovery/logout/stop cycles pass, with Telnet reconnect and shell
  recovery. An earlier complete three-cycle run hit an immediate Telnet reconnect
  before listener rearm; the harness now allows the documented reconnect delay.
- Native system free/largest after every stop:66302/66170, with both services
  stopped; after Telnet restart:45602/45470. Final two matched app free/largest
  samples322392/322016; earlier small allocation-layout variations are retained
  in raw evidence. Runtime remaining command allocations0. Fault-probe app
  allocations return exactly, independently of shell/history bookkeeping.
- Full sealed build, offline HDA/tar fork/metadata readback and normal installed
  Core/Seriald byte comparison pass. Core update/relaunch succeeded on a fresh
  clone with remote services stopped before handoff.

## Cost and limitations

Seriald file31736→31776bytes (+40); loaded image22288bytes unchanged. Telnetd
byte-identical. Core440064bytes and all standard utility sizes unchanged.
No new persistent state, buffers, idle work or additional native allocation.
A permanently failing driver keeps the foreground owner/cleanup waiting; forced
quit or application crashes are not made safe by this fix. Physical driver
failure behaviour remains untested.

One earlier Core handoff with a remote session connected halted Snow at PC0010000C.
The corrected Seriald service was not running. Failed image/log preserved locally;
no updater correction or claim of resolution. Fresh-clone handoff with services
stopped passed. This observation is outside the narrow Seriald correction.

Publication verification is recorded in verification.json after integration;
prior1974 distribution/recovery and unrelated work remain preserved. Stop4E.
