# PB180 self-update hardware test — 18 November 2026

User authorized physical PB180 control, update and testing. Target 192.168.1.132;
SE excluded. System API 0710 and CPU code 4 observed. Open Transport and older
SEControl are user-reported; exact networking/helper versions not yet verified.
Control uses the existing paired SEControl on host port 5968, separate from the
SE listener on 5969. No network settings or helper binary changed.

Before update: free application heap 378.91 KiB, launch 380.72 KiB, System 7.1.
Installed updater is repository-capable. Full repository update to sealed build
2026111902 started through that command using the private allowed-peer HTTP
server on port 8098, path /self-update-1902/repository/index.txt. Standard native
replacement retains recovery copies. Existing user-facing feeds are unchanged.

Core 1882→1902 completed, relaunched automatically and printed “Clarice update
complete.” SEControl connection survived the handoff with no helper upgrade,
manual Clarice restart, OS reset or network reconfiguration. New banner identifies
PowerBook 180, CPU 68030, FPU 68882, RAM 10 MB, 640×400 4-bit, System 7.1.0.
Builtin/repeat checks below complete; screenshots are physical evidence.

`command -v clarice` resolves to the builtin. Initial post-finish runtime reports
heap 386152, largest 385908, updater image 100604, module ticks 105/load72.
Those ticks cover startup finish, not full download/handoff. `telnetd` refuses
because saved authentication settings are missing/invalid; left unchanged.
Validation continues via SEControl rather than weakening authentication.

## Results

- Stale `/bin/clarice` moved to `/etc/clarice/legacy-updater-1882.clx`; retained,
  not deleted. Builtin resolves before and works after removal. Post-update
  `NO_EXTERNAL=0` and `NO_PENDING=0` prove no external updater was reinstalled
  and no incomplete marker remains.
- First/second measured bundled help: 0.31 / 0.26 guest seconds, status 0.
  These are not cold launches: startup completion had already used the resource.
  Free heap inside the script 367960 / 367976 bytes; largest 367736 / 367864.
- Complete no-change repository update: 3.10 guest seconds, status 0, reports
  all standard tools current. Script free heap remains 367976, largest 367820.
  Updater module ticks 181, loader12; these omit resource acquisition and are
  not substituted for the whole-command time. Final idle after script and
  log-display command: free heap386412, largest382936. This is bounded cleanup
  evidence, not full working-set or system-heap profiling.
- Sort produces a,b and wc reports 2 lines for the disposable input. Script
  reaches PB_END. Application stays responsive at the prompt.
- Existing SEControl captured 640×400 successfully throughout; no reconnect or
  manual intervention during application handoff. Helper exact version and Open
  Transport version remain unverified. The installed stack's HTTP path works.
- Telnet was not tested: its enable attempt reported missing/invalid password
  settings. No configuration or credentials were changed; it remains disabled.

The initial physical Core handoff was initiated by the existing external updater;
startup completion and subsequent commands ran the new builtin. A further Core
handoff initiated by the builtin is covered by Snow, not by this one-version
hardware run. Fault-injection, disk-full, power loss, and long endurance are not
claimed. User's SE and other agents' instances were untouched. No production
code changes were needed. Standard recovery app/tool copies remain on the PB180.

Private test repository stays on port8098 at /self-update-1902/repository/index.txt;
the user's saved default source and existing feeds are unchanged. No legacy
migration feed is maintained: the full repository worked directly. The temporary
host control listener is stopped at test completion, leaving Clarice open.
