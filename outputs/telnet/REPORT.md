# Native Telnet client — 17 November 2026

**Current implementation:** see [VT100/ANSI acceptance](#vt100ansi-acceptance).
The following NVT and real-shell sections record the earlier baseline; their
TERM=dumb/no-full-screen limitations are superseded only as described below.

Independent networking branch; no0.5 RC, FTP, Core or renderer changes.
Build `sh scripts/build-telnet.sh`: `work/telnet/telnet.clx`,9,944 bytes,
SHA256748a4584149b17f393d8f66b51bd57de9ce5ea51278992066b6b905c7dbbd0a3.
68000 runtime1.1 with existing imports. Runtime image6,860bytes and one existing
16KiB native receive ring while connected; no permanent Core allocation.

## Usage and compatibility

`telnet HOST [PORT]` (default23); `telnet --help` describes controls/limits.
Ctrl+] then q exits locally, doubled Ctrl+] sends that character. Ctrl+C sends
Telnet Interrupt Process; Command-period cancels locally. Existing configured
Control substitution applies (Option on the Plus). Return sends NVT CR LF;
Backspace sends DEL. Uses system DNS and installed native TCP.

Character-mode NVT with server echo negotiation, SGA, terminal type DUMB,
fragmented IAC/subnegotiation handling, CR NUL and escaped IAC. Unsupported
options refused. Passive negotiation only, with state per direction: duplicate
enabled/disabled requests do not cause acknowledgement loops. No locally
pending option requests, so a general Q-method state machine is unnecessary.
The existing serial remote-tty API owns keys/rendering/restoration.

Not full VT100/ANSI, a local line editor, SSH/TLS, login manager, transfer client
or Telnet server. Both stdin/stdout must be tty. ASCII/NVT is the supported text
contract; no negotiated character-set conversion. DUMB is the protocol terminal
name, not a claim of curses/full-screen support. Plaintext credentials/data.
Ctrl+C sends IP without TCP urgent SYNCH; servers requiring SYNCH are unverified.
Network polling cannot distinguish all reset/error closures from graceful EOF
with the current shared NetAvailable interface; ordinary EOF returns0. Other
errors return1, usage2, local cancellation130. Native connect/send/close and DNR
callback drain can delay cancellation; no idle-session timeout. A control that
remains incomplete30seconds, a subnegotiation beyond4096bytes or keyboard queue
overflow fails explicitly. Only two subnegotiation bytes are retained. Receive
and display batches512bytes, transmit256; command state is invocation-local.

## Actual tests

ASan/UBSan `tests/telnet/run-host.sh`: fragmented NVT/IAC, echo transitions and
duplicates, TTYPE, option refusal, CRLF/CRNUL, IAC quoting, IP, malformed and
oversized controls, protocol timeout, EOF, cancel/escape, keyboard overflow,
connect/send/output errors, help/port validation and20 restoration cycles pass.
FTP sanitizer regression unchanged and passing.

Snow Plus/4MB/System6.0.8 with MoofTCP/DaynaPORT, normal speed for input/network
scheduling: independent telnetlib3 server interoperability, single echo,
Backspace editing, remote IP via Option+C,100-line burst through final row099,
Ctrl+] equivalent Option+] then q, reconnection, Command-period cancellation
and subsequent `status runtime` pass. First attempt correctly failed while the
host fixture had a startup API error; corrected the fixture, no client patch.
Snow physical Control automation produced plain c; switched to the already
supported Plus Option mapping rather than changing keyboard code. Terminal
restored; remaining0, largest400,128; repeated warm heap400,598/400,600.
Screens and `snow-server.log` retain actual outcomes including failed automation.

The fixture is **TEST>**, not an OS shell: it executes no programs. It uses
unmodified telnetlib3 5.0.2 for protocol negotiation and provides echo, erase,
interrupt, burst and peer-close tests. No remote Unix login/full shell acceptance
is claimed. `tests/telnet/server.py --bind HOST --peer ALLOWED_IP` restricts peers;
`--port 2324 --auto-close` supports repeated lifecycle testing. Host-only Python
dependency, none added to Clarice. System7/OpenTransport and broader server
interoperability remain untested.

## References

Independent implementation; no BusyBox source transplanted. Studied Tomi Ollila
and contributors' [BusyBox Telnet](https://github.com/mirror/busybox/blob/master/networking/telnet.c)
(GPL2+), [RFC854](https://www.rfc-editor.org/rfc/rfc854) and
[RFC1143](https://www.rfc-editor.org/rfc/rfc1143). Native transport and existing
serial services reused. [telnetlib3 documentation](https://telnetlib3.readthedocs.io/en/latest/api/server.html)
guided the independent interoperability fixture. No new sockets/runtime layer.

## Physical SE acceptance

Same stock 68000/4 MB/System 6.0.8 reference setup and installed networking as
FTP acceptance; Core1550 unchanged. Downloaded only /tmp/telnet-se and the
fixture script. Three independent connect/negotiation/output/peer-close sessions
all returned0. Server recorded TTYPE DUMB. Free application heap
397,434/397,432/397,434; largest397,380 throughout; remaining0. Total lifecycle
128/149/139 native ticks (2.13/2.48/2.32s), including load36/33/33ticks. These
include the fixture's negotiation scheduling and peer closure, not a pure input
latency benchmark. Driver system-heap peak/reclamation was not directly measured.

Physical interactive acceptance: single echo, 65-character host burst received
exactly (SEControl submission5.13s, not maximum SE throughput), remote Ctrl+C/IP,
Backspace/DEL editing `echo bad` to `echo good`,100-line burst through row099,
Ctrl+] then q local exit, hostname-based reconnect, Command-period local cancel
status130, and restored shell/runtime status. Logs/screens and
`se-input-check.json` retain evidence. No precise key-to-pixel latency claim.
Post-cancel heap402,672/largest398,880, remaining0 outside the script allocation
context. Existing FTP uploaded the953-byte result log successfully afterwards;
FTP executable hash unchanged and its sanitizer regression passed.

No demonstrated product failure required follow-up edits. Snow stopped and its
profile reset; temporary host services stopped; physical Core/settings/bin left
unchanged and SEControl retained. Test CLX remains under /tmp. No push/release.
This does not claim login to a production Unix telnetd, arbitrary ANSI/BBS or
all four physical network pairings. Test service cannot execute MacBook programs.

## Real Unix shell follow-up — 17 November 2026

Completed after the fixture-only acceptance above. Same unchanged Telnet CLX
SHA256; no Core, FTP, installed utilities or release-candidate changes. No rebuild
was necessary. Added `tests/telnet/shell.py`: existing telnetlib3 protocol support
bridged to a real macOS PTY and `/bin/bash --restricted --noediting -i`, using the
serial-shell fixture's disposable directory, minimal environment/PATH and
`TERM=dumb`. `MACBOOK>` identifies the remote shell; Clarice's `/ >` is local.
Restricted Bash is not a security sandbox. Temporary trusted-LAN listener accepts
only explicitly listed peers, one session at a time, bounded reads/backpressure
and a session deadline. It is test infrastructure, not a Clarice Telnet server.

Run with the existing host telnetlib3 environment:

```
python tests/telnet/shell.py --bind MACBOOK_IP --peer SE_IP --seconds 300
```

For Snow NAT, also allow the observed NAT source (the MacBook address in this
setup). Connect `/tmp/telnet MACBOOK_IP 2323` in Snow or `/tmp/telnet-se ...` on SE.
Exercise `pwd`, `ls`, `echo`, `cat sample.txt | wc -c` (15), Backspace correction,
`sleep 30` followed by Ctrl+C, Ctrl+] then q, reconnect, Command-period during
another sleep, reconnect and `echo recovered; exit`. Inspect `echo $?` and
`status runtime` locally after each exit. No credentials or production login used.

Actual Snow Plus/4MB/6.0.8 MoofTCP/DaynaPORT tests at normal emulator speed and
physical stock SE/4MB/6.0.8 tests passed these scenarios. Remote single echo,
Return, Backspace, pipeline output and real foreground-process SIGINT worked.
Snow used its existing Option substitution for Control. Initial automated shifted
characters arrived unshifted; explicit physical modifier events corrected the
pipeline and status checks, without product changes. An initial host test used
an incorrect telnetlib3 IP-sending method; corrected to `send_iac(IAC+IP)` before
guest acceptance. Raw logs preserve these attempts.

| Measurement after session | Snow | Physical SE |
|---|---:|---:|
| Warm application heap, three consecutive measured exits | 403,270 / 403,270 / 403,270 B | 402,672 / 402,672 / 402,672 B |
| Largest free block | 399,076 B | 398,880 B |
| Tracked remaining allocations | 0 | 0 |
| Normal remote exit | 0 | 0 |
| Local Command-period cancellation | 130 | 130 |

Physical local Ctrl+] exit returned0. All sessions restored the local terminal
and subsequent commands. These are functional/stability checks, not throughput
benchmarks: session duration includes typing, waits and screenshots. Native driver
system-heap allocations were not measured. Nine host PTY sessions (including two
host fixture checks) were reaped; no test shell/sleep process remained. Server and
Snow stopped; emulator test profile reset; SEControl and physical Clarice remain
running. [Results](shell/results.json), [wire/PTY log](shell/server.log) and
`shell/*-interrupt.png`, `*-cancel.png`, `*-recovered.png` retain actual evidence.

This establishes a real Unix shell over NVT, not arbitrary telnetd/login-server,
urgent SYNCH, ANSI/VT100, curses, System7 or OpenTransport acceptance. No additional
terminal emulation or production fix was justified by these tests.

## VT100/ANSI acceptance

External utility only: `telnet [--dumb] HOST [PORT]`. Default advertises VT100;
`--dumb` retains the accepted NVT behaviour. Current CLX is **17,924 bytes**,
SHA256 `8b13c31f4cc25672fc3ec2f876e1411895480c5837441c626e84e187df72ce16`.
Build: `sh scripts/build-telnet.sh`; artifact `work/telnet/telnet.clx`. Existing
Core1550 loads it unchanged. No release-candidate, Core, FTP, installed `/bin`,
nano or renderer changes; physical candidate remains `/tmp/telnet-vt2`.

### Implementation and bounds

`src/commands/terminal/vt100.h` independently implements the remote character
model using Clarice's existing `CliScreen` storage/erase helpers. Telnet sends
changed rows and cursor/attribute updates through `app_screen_open` and `cli_write`;
the existing nano screen service owns QuickDraw, clipping, theme, keyboard and
restoration. No duplicated pixel renderer, bitmap cache or permanent allocation.
The extra model is necessary because the existing external screen API does not
expose scrolling-region/cell manipulation; this avoids changing the accepted
Core ABI or deploying a new Core merely for a client feature.

Supported: cursor positioning/movement/save/restore, erasure, delayed autowrap,
origin mode, scrolling regions/index/reverse index, insert/delete characters and
lines, tabs, inverse video/global reverse, cursor visibility, application arrows,
device/status/cursor replies, G0/G1 DEC graphics with ASCII approximations and
Telnet NAWS dimensions. Full-screen input uses the existing application queue;
Ctrl+] then q disconnects, Command-period cancels, Ctrl+C sends Telnet IP.
The host PTY translates IP into VINTR, appropriate to its current terminal mode.

Limits are explicit in `--help`: monochrome ASCII, no UTF-8, colour,
bold/underline/blink, double-size lines, VT52, alternate-screen history or
application numeric keypad/function-key translation. Do not advertise xterm or
assume full VT conformance. Size is fixed for a session (the existing screen
service prevents resizing while owned); resize before connecting. No SSH/TLS,
TCP urgent SYNCH or remote authentication added. No scrollback/selection within
the remote full-screen surface; local shell history is restored on exit.

OSC/DCS strings are discarded, including clipboard/title requests; oversized
controls fail after4096bytes, CSI parameters saturate with bounded storage.
Input/output/protocol failure, acquisition failure and cancellation release
owned state. Burst rendering consumes up to4KiB before presenting a frame,
with keyboard/cancellation checks every512received bytes. This avoids painting
every network chunk; final partial batches are always presented.

Measured layout sizes with the 68000 compiler: model **11,116 B**, existing
on-demand Core screen **10,804 B**, runtime image **12,612 B**, plus existing
**16,384 B** native receive ring. These components total50,916 B while connected,
excluding allocator/driver overhead; this is a bounded component accounting,
not a measured all-in peak. Compared with NVT's image/ring, roughly27KiB additional
invocation memory, **no idle allocation**. Core screen and model are separate
text states, not additional bitmaps. Port `owned-peak=0` does not account for
these native allocations; cleanup evidence includes actual free application heap.

### Validation actually executed

Host ASan/UBSan: legacy NVT cases retained; VT model tests cover regions/wrap,
origin/CPR, save/restore, insert/delete, graphics, malformed/string controls,
200,000 fuzz bytes. Negotiation tests check VT100 TTYPE and escaped NAWS encoding;
input tests check application arrows. Dirty-buffer comparison feeds generated
rows through the unchanged Core parser and compares cells, attributes and cursor.
Allocation/acquisition failure, failed output/input, cancellation and20 repeated
VT acquisitions leave zero mock allocations/owners. Existing Core screen tests
(100,000 parser bytes) also pass. No broad unrelated suite repeated.

Snow Plus/4MB/actual6.0.8 MoofTCP/DaynaPORT, then stock SE/4MB/6.0.8:

- Real macOS **Vim9.1** opened, edited and saved `sample.txt`; host contents verified
  exactly `snow-vt remote fixture\n` and `se-vt remote fixture\n` respectively.
- Vim page-down/search reached line090 of120; physical arrows arrived as
  `ESC O B` / `ESC O C`, matching application mode.
- **less668** paged and searched to line110, with correct inverse search/END
  highlighting and complete final screen.
- Cancellation from full-screen less returned130 and restored Clarice; reconnect
  and remote exit returned0. Physical Ctrl+] local escape returned0.
- Warm Snow heap403,262/largest401,056 stable across two final exits. Physical
  heap402,672/largest398,880 unchanged across cancel, remote exit and local escape;
  tracked remaining0. Native driver system heap was not measured.
- Physical CLX load53/52/54ticks (about0.87–0.90s); interactive session totals
  include human/automation waits and are not execution/throughput benchmarks.
  No quantitative redraw-speed or flicker claim from sparse screenshots.

[Machine-readable evidence](vt/results.json), [host PTY log](vt/server.log),
[SE Vim save](vt/se-saved.png), [SE Vim search](vt/se-search.png),
[SE less](vt/se-less.png), [SE cleanup](vt/se-final.png),
[Snow less](vt/less-final.png) and [Snow recovery](vt/final-recovery.png).
Seven host sessions were reaped; temporary servers and Snow stopped. Physical
Clarice/SEControl remain running; previous Telnet and recovery copies untouched.

Fixture corrections: raised telnetlib3 negotiation deadline from1to10seconds
(the earlier deadline started a dumb shell before slow guest negotiation finished).
Snow's Plus mapping did not deliver host Escape/arrows, and naive synthetic Shift
lost uppercase/punctuation. Used explicit modifiers and Option+[ for Escape;
physical Escape/arrows worked. Initial failed automation and the earlier less
partial-frame capture remain in logs, not counted as passed tests. Actual host
`nano` is a symlink to Pico, so no remote GNU nano acceptance is claimed.

### Research and remaining coverage

Consulted DEC's [VT100 programmer manual](https://oldmanprogrammer.net/webapps/vt100/doc/vt100-ug/chapter3.html),
[libvterm parser](https://github.com/neovim/libvterm/blob/master/src/parser.c)
(control/string separation; MIT project, not copied or linked), and
[RFC1073 NAWS](https://www.rfc-editor.org/rfc/rfc1073.txt).
The existing bounded model/native renderer were the smallest reusable foundation;
no library dependency or Core protocol change was needed. This is focused
Vim/less/PTY acceptance, not arbitrary telnetd/login, vttest certification,
System7/OpenTransport or colour/full-xterm coverage. FTP binary hash is unchanged.
