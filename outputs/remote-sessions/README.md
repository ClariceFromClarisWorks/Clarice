# Single supplementary Telnet session — Snow acceptance

18 November 2026. Core **2026111810**, optional `telnetd` **2026111811**.
Working tree only; no push/release or physical-SE access. The updater checkpoint
0891878 and unrelated collection experiments remain preserved.

## Implemented

- Genuine existing libtelnet protocol engine, unchanged upstream source/licence;
  existing mrsh executor and external GNU/sbase utilities. Native MacTCP passive
  open, asynchronous sending, bounded receive polling; no socket/PTY subsystem.
- One optional external service image retained only while enabled. The existing
  loader validates it. Stop releases the image only after native I/O ownership is
  released; an incomplete abort retains storage for a safe retry.
- Independent directory, exported environment, variables, functions, arguments,
  streams, exit status and cancellation. Subshell exit does not log out its parent.
- One executing command at a time. Local editing/history/scrolling remain usable
  during remote commands. Local Return queues submission until remote completion;
  network input is serviced during local commands, with remote execution deferred.
- Remote line editing: backspace, Ctrl-U, last-command Up/Down, Ctrl-D at an empty
  line, 1,023-byte command lines, 4 KiB pending input. Overflow reports an error and
  never executes a truncated line. CRLF, negotiated echo, Telnet IP/BREAK/EC/EL.
- Standard streams, direct native text and progress output remain on the originating
  terminal. GUI editors and local-input clients are rejected remotely. Self-update
  remains local because application handoff would destroy the active service.

## Usage and boundaries

The service is **off at launch**, with no networking allocations while disabled.
Open **Preferences → Telnet server…**, set the permitted client's IPv4 address,
port and password, check **Enable now**, then **Apply**. Blank password keeps the
existing password. Apply disconnects any current remote session; Cancel changes
nothing. Settings survive restart; enabling does not (explicitly labelled).

```
telnetd --preferences   # same native settings dialog
telnetd                 # enable using saved authenticated settings
remote status
remote off
telnetd --help
```

The address is the client's address, not the Macintosh's. The optional service
must first be installed; Core does not bundle it or silently install it.
The explicit test-only alternative remains
`telnetd --no-password --peer 192.168.1.112 --port 2323`.

`/etc/clarice/telnet.conf` contains version, peer, port, work factor, salt and
verifier (under 256 B), never plaintext. Existing native safe replacement preserves
an intact or recoverable previous file on save failure. Malformed/missing settings
fail closed. Passwords are 1–127 printable ASCII bytes. Password entry is masked
and does not use Scrap, argv or command history. No fake Unix user accounts.

Authentication uses genuine [fastpbkdf2](https://github.com/ctz/fastpbkdf2), Joseph
Birr-Pixton, CC0; upstream source/license retained under
`src/commands/remote/vendor/fastpbkdf2`. SHA256 is the existing public-domain libutil
implementation. Thin adapters remove OpenSSL and unused hashes, use alignment-safe
stores, clear temporary key state and yield every 16 rounds. No crypto links into
Core. 1,000 rounds is a **legacy CPU compromise**, not modern offline password
hardening. Salt uniqueness uses time/ticks and previous salt, not a claimed CSPRNG.
HFS supplies no password-file permissions. Use a unique strong password.

Telnet **does not encrypt** passwords or traffic; trusted-peer filtering is not
cryptographic authentication. Use a trusted private network. No shell is created
until verification succeeds; command bytes pipelined with a password are discarded.
Login times out after 60 guest seconds, failed logins close and impose a five-second
cooldown across reconnects. Stop before upgrading the loaded service package.
Core handoff safely stops it; restart does not automatically enable it.
Serial remote sessions, transfers and remote full-screen editors remain separate.

## Actual validation

Accelerated Snow: Macintosh Plus, 68000, 4 MiB, **System 6.0.8**, fixed maximum
speed (~15× observed). MacTCP/.IPP. No MoofTCP or physical-hardware acceptance.

- `auth-client.py` / `auth-snow.json`: wrong/empty/overlong rejection, no echo or
  pre-auth execution, three authenticated reconnects on final service 1811.
- `auth-preferences.json` and screenshot: masked entry, saved port change and
  restoration, password replacement rejects old/accepts new, settings persist
  through Core update/relaunch; no plaintext in config or history.
- `auth-config-recovery.json`: locked settings save stops safely and preserves the
  original byte-for-byte; old password reconnects after unlocking.
- `auth-recovery.json`: malformed config refuses startup; saved copy restores it;
  three enable/stop cycles and pre-auth Ctrl+C/reconnect. Initial exact-heap assertion
  encountered the known reversible two-byte variation, not a progressive leak;
  test now checks return to baseline, fixed largest block and at most that variation.
- `auth-host.txt`: ASan/UBSan, PBKDF2 comparison with Python hashlib, cancellation,
  password gating, discarded pre-auth input, config parsing and ownership checks.
- `client.py`: seven inbound command checks: cwd, persistent environment/functions/
  arguments, pipeline, file redirection/readback and runtime statistics.
- `interaction.py`: overflowing input cannot create its prefix's output file;
  multiple submitted lines execute in order; local typing at 25 key pairs/second
  and scrollback work during a remote busy loop; queued local command executes
  after remote interruption; remote canonical `read`; repeated pipelines/reconnects.
  Screenshots confirm local `/` prompt remains local while remote cwd is `/tmp`.
- `lifecycle.py`: second client gets no shell; ETX and IAC IP cancellation; local
  Command-period does not cancel remote execution; disconnect during continuous
  output unwinds and reconnects; child exit and parent logout behave separately.
  A final logout test caught dropped pending stdout; 1807 now drains writes and
  performs a bounded async close. `logout-output.json` confirms final output reaches
  the client before EOF.
- Host ASan/UBSan: session isolation, UI context switch/restore, nested exit,
  read/write/parser failures, cancellation, 80 execution and 12 session-open
  allocation-failure points. Service mock: negotiated echo, CRLF, queued lines,
  overflow, delayed abort (buffers retained), reconnect, zero retained allocations.
- Existing local shell comparisons **31/31**, cancellation **3/3**. Import lookup
  checks pass. Normal Core build validates all 32 standard images/47 command names.

Authenticated regressions: `auth-commands.json`, `auth-interaction.json`,
`auth-lifecycle.json`. Earlier baseline evidence: `inbound.json`, `interaction.json`, `lifecycle.json`,
`enable-stop-memory.json`, host text/JSON results and adjacent screenshots.

## Measured cost and cleanup

- Optional CLX file **37,248 B**; enabled image **27,928 B** (5,896 B more than
  unauthenticated 1807). Native driver storage remains **20,692 B** in system heap.
- Session/protocol allocations remain demand-owned; KDF uses transient stack space,
  no heap/cache. Stop releases image and native storage. Three final enable/stop
  cycles: heap **365156, 365154, 365156**; largest **363042** throughout. System-heap
  release is proven by host ownership tests, not a guest system-wide heap measure.
- Core MacBinary **276,736 B**, +1,408 B versus 1804; package growth is not a
  permanent RAM or startup measurement. Disabled Core does not read Telnet config,
  initialize networking or allocate a service; UI has one transient password pointer.
- Final authenticated reconnects: **1.42–1.47 s host elapsed**, fixed maximum Snow
  acceleration (~15×). Not guest CPU timings or a prediction of physical-SE speed.
  Physical password-verification cost is explicitly pending.

## Reproduce and deploy

`sh tests/remote-sessions/run-host.sh` runs focused ownership/protocol-service mocks.
`sh scripts/build-telnetd.sh` freshly builds the optional runtime-1.8 CLX; failed
builds remove the old publishable output. Existing complete Core build stays normal.

Core was published using `scripts/repository.py`, then installed with
`clarice update -y --source http://192.168.1.112:8083/remote/index.txt`, including
automatic relaunch. `tests/remote-sessions/publish.py` adds the optional image to
that same repository format using immutable content and a new version. Subsequent
`clarice update -y` installed service versions 1809 and 1811 after Core 1808/1810, retaining the
previous utility for recovery. No manual executable copies into guest disks.
A Core with runtime 1.8 must be installed before adding this optional package;
old Core cannot validate its new imports. Final check found no pending upgrades,
and the service was stopped.

### Inbound Snow test path

The existing NAT lacked inbound forwarding; raw bridge required unavailable BPF
permissions. A small opt-in patch reuses Snow's existing smoltcp/NAT forwarding and
cleanup, binding **only 127.0.0.1**. No macOS permissions or LAN configuration changed.
See `tests/remote-sessions/snow-loopback-forward.patch` and `SNOW-LICENSE` (Snow,
Thomas W., MIT). Apply to the existing development Snow source, build its normal
frontend, and launch the isolated test workspace with:

```
SNOW_TELNET_FORWARD=23231,10.0.0.2:2323 path/to/Snow test.snoww
```

In this guest, set peer **10.0.0.1** in Preferences; the NAT gateway is the
client endpoint. Configure a disposable test password locally. Supply it to the
host tests via `CLARICE_TEST_PASSWORD` without adding it to tracked files. Host tools use Python 3.9's established `telnetlib`, connecting
only to `127.0.0.1:23231`:

```
python3 tests/remote-sessions/client.py
python3 tests/remote-sessions/interaction.py SNOW_PID
python3 tests/remote-sessions/lifecycle.py SNOW_PID
python3 tests/remote-sessions/auth-client.py
python3 tests/remote-sessions/auth-recovery.py SNOW_PID
```

UI tests intentionally activate only that Snow PID. The forwarding patch is
external development infrastructure, not Clarice production networking.
