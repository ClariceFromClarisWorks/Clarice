# Physical-SE performance tools

External test tooling only; no production RAM or CPU cost. Run from the isolated
performance checkout. Existing Snow tests remain unchanged.

- `overall.sh`: local complete-command timing and exact-copy verification.
  Requires latest utilities; refuses reuse of its disposable fixture directory.
- `remote.py --output FILE`: authenticated SE Telnet command/cancellation/repeat
  checks. Set CLARICE_TEST_PASSWORD without exposing it in logs.
- `serial.py --pty /dev/cu.usbmodem1401 --output FILE`: explicit physical serial
  login/interrupted-transfer/cleanup checks at 57600 8N1, paced 2 ms per byte.
  Restores host termios. Cable removal itself must be separately observed.
- `kermit.py --peer /absolute/path/wermit --transport telnet|serial --output FILE`:
  genuine C-Kermit interoperability, binary/MacBinary container exact readback.
  Default SE192.168.1.252:2323; explicit USB modem1401. Existing targets are
  refused, not removed; use a new named fixture for another completed run.
- `measure-nano.py --socket ABS --variant b|a`: matched local diagnostic editor
  runs, no edits/saves; retain screenshots and action logs. Install the intended
  variant as `nano-profile` first, preserve every prior recovery file, and disable
  remote services. Default cases include first plus three repeats for no-file
  and ordinary8KiB, plus existing-empty and long-line controls.
- `collect-nano.py --password-file FILE --output FILE`: after the local runs,
  enable seriald and collect exit status, startup phases and allocation counters.
  Fails on missing markers, nonzero editor exits or tracked survivors. Password
  file stays private and untracked. Mark0 follows command echo repaint; mark8
  follows first draw, before waiting for input. This interval excludes command
  echo and input delivery, so cannot alone certify the full three-second goal.
- `build-network-diag.sh pending|fixed-peer-port`: opt-in native passive-open
  diagnostic, separate from the normal service. Fixed-source-port evidence does
  not certify ordinary clients. No authentication or peer-policy relaxation.
- `integration1841.py`: disposable physical locked-source, empty-mkdir, cwd
  relocation, remote-input diagnostic and cursor checks after the concurrent
  development integration. Uses the private ignored password file and refuses
  an existing fixture directory; unlocks its own source in a finally block.

Host elapsed times include transport and authentication where stated. Local
`time` includes command loading. Guest counters and video are separate evidence.
Scripts failing do not constitute hardware acceptance. For memory stability,
retain all samples and inspect warm-up/trends instead of hiding noisy samples.

Reuses native Clarice time/status and upstream C-Kermit; no custom benchmark
runtime, cache, assembly, or additional production instrumentation required.
Smallest useful measurements are first plus three warm local runs, ten remote
command cycles and three reconnects. No claim of long-term endurance.

USB CDC host handoff must set CLOCAL/CREAD and assert DTR/RTS explicitly;
a short settle interval precedes the first Return. Do not attribute the initial
C-Kermit login-prompt failure solely to SE firmware or DTR. The independent
serial acceptance passed before that handoff; retesting after cable reconnect.

- `peer-access.py allow|deny --host HOST --port PORT --output FILE`: checks the
  approved wildcard listener after configuring the guest's allowed peer separately.
  Allow mode requires `--password-file FILE`; deny mode requires EOF/reset with
  zero bytes, not a timeout. Default target is the isolated Snow loopback23291.
  Restore the original peer configuration before the allowed recovery check.
