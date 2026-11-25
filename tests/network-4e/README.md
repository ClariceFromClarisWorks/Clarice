# Seriald startup ownership regression

`sh tests/network-4c/run-host.sh` includes the extended actual Seriald host tests:
20 repetitions each of output-open, reset and registration failure, combined
with repeated input-close/output-close and buffer-detach failures. Retry waits
return cancellation while asserting a live owner. Successful cleanup is repeated
and asserted harmless. The preceding implementation fails the zero-owner check.
The established service tests retain delayed async writes/abort and login coverage.

`sh tests/network-4e/build-guest.sh` builds a disposable normal68000 Snow probe.
It calls the real Serial Driver, injecting errors *before* selected native calls.
Only config reads and registration are fixtures. Five repetitions of all three
startup paths check driver ownership, zero application/system heap deltas and
idempotent teardown. It is not included in distribution packages. Run on an unused
emulated modem port with Seriald disabled; no physical hardware is required.
The normal optional Seriald binary is separately tested for login, command/read
cancellation, logout, stop and restart through Snow's modem PTY.
