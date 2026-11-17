# Telnet protocol review — 2026-11-17

Decision: retain the SE-validated client for its current scope. Prefer a genuine,
no-zlib libtelnet port over extending our two-state negotiation into a general
client/server engine when proactive negotiation or server work is authorised.
Do not port BusyBox or transplant NCSA. No production implementation changed.
MacTCP/MoofTCP, DNR, terminal services and VT renderer remain appropriate.
The SE is shut down; no hardware interaction was attempted during this review.

## Source and attribution

- [libtelnet](https://github.com/seanmiddleditch/libtelnet), commit
  `5f5ecee776b9bdaa4e981e5f807079a9c79d633e` (develop): libtelnet.c/h,
  upstream RFC1143/TTYPE and other test fixtures. Sean Middleditch and Jack Kelly;
  public-domain dedication in COPYING. Repository archived: adoption entails
  maintaining a pinned upstream plus isolated platform adaptations.
- [BusyBox telnet.c](https://github.com/mirror/busybox/blob/master/networking/telnet.c),
  retrieved source SHA256 `cdc756f6d3eeaa77393b2920654791b86a6d7eaba37dc651424c9191de97626e`.
  Tomi Ollila and subsequent contributors; GPLv2-or-later. Inspected option,
  subnegotiation, terminal-mode and poll paths, not a complete BusyBox port.
- [NCSA Macintosh 2.7b5 source](https://www.macintoshrepository.org/83-ncsa-telnet-2-x),
  reused prior extracted source under Clarice/work/terminal-history/read/ncsa:
  source/parse/parse.c, particularly parse, SendNAWSinfo and telnet_do/will/wont.
  Historical NCSA contributors retain attribution; no source copied. Redistribution
  would require checking the selected archive's complete notices. BetterTelnet's
  separate GPL licence is not assumed to license every NCSA release.
- Protocol references: [RFC854](https://www.rfc-editor.org/rfc/rfc854),
  [RFC855](https://www.rfc-editor.org/rfc/rfc855),
  [RFC1143](https://www.rfc-editor.org/rfc/rfc1143).

## Correctness and maintainability

| Area | Current Clarice | Comparison / conclusion |
|---|---|---|
| Negotiation | Passive only; separate local/remote bits; suppresses redundant positive acknowledgements and already-disabled negative replies; rejects unsupported options | Adequate for this restricted state space. Not a general RFC1143 engine. libtelnet handles pending/opposite/crossed requests and is the clear reuse choice before proactive negotiation. No reason to implement those states ourselves. |
| TTYPE | Answers exact SEND only after accepted DO; VT100 or DUMB | Correct for one advertised type. libtelnet provides typed events; caller still chooses truthful type. BusyBox's scanner is less strict about the SEND sequence. |
| NAWS | Correct 16-bit dimensions and doubled IAC; fixed for screen-owned session | NCSA SendNAWSinfo clamps 255 to 254 and transmits low-byte dimensions; no advantage to transplanting it. Library still needs application dimension/escaping integration. |
| Echo/SGA | Negotiated remote echo, local echo otherwise; character-at-a-time | No canonical local line discipline/GA turn-taking. BusyBox uses Unix termios; NCSA has a separate line-mode subsystem. Neither is a small native-Mac drop-in. |
| BINARY | Refused in both directions | Safe negotiation refusal, but not binary-transparent or a complete 8-bit client. libtelnet tracks directional binary modes; renderer/keyboard character-set policy remains separate. |
| NVT/control | CR LF/CR NUL reception, doubled IAC, Return CR LF; IP on Ctrl+C | No urgent TCP SYNCH, AO/AYT/EL UI action; EC currently moves back rather than fully implementing erase-character semantics. Do not claim full RFC854 control coverage. A library does not supply urgent transport or terminal policy. |
| Subnegotiation | Two retained bytes, 4096-byte wire cap, malformed/unfinished sequence failure, 30-second deadline | Bounded and allocation-free. libtelnet retains payloads up to 16 KiB and parses additional ENVIRON/ZMP/MSSP forms even when not useful here. Native adapter must map warnings/failures and enforce cancellation/deadlines. |
| Cancellation | Local escape, Command-period, bounded receive batches; native send timeout | Transport cancellation and cleanup stay with Clarice. None of the alternative parsers replaces native lifecycle handling. |

No evidence justifies replacing the working full-screen path. Current code is
compact but densely formatted and mixes protocol and presentation; a future
library adapter should separate those responsibilities without rewriting either
transport or renderer. NCSA's inspected subnegotiation append sites have no local
bounds check, so historical provenance alone is not evidence of safer parsing.

## Costs and actual measurements

Current complete CLX remains **17,924 bytes**, runtime image **12,612 bytes**.
Protocol uses two 32-byte option bitsets and a two-byte retained suboption; it
performs no protocol-specific heap allocation. Complete client's compiled object
text is 9,386 bytes, BSS 1,436 bytes, including VT logic and I/O buffers.
Existing connected storage includes VT model 11,116 bytes, on-demand Core screen
10,804 bytes and native receive ring 16,384 bytes. These are component sizes,
not an all-in measured peak; prior physical cleanup evidence remains valid.

Isolated upstream libtelnet, same Retro68 `-m68000 -Os`, no zlib,
function/data sections, relocatable link GC retaining init/free/recv/send/
negotiate/subnegotiation: **5,875 text bytes**, no static data/BSS. This is an
engine-only figure, NOT a replacement-client executable or a size delta.
It still references calloc/malloc/realloc/free, memcpy, strlen, strerror,
errno and vsnprintf; adapter and runtime dependency costs are unmeasured.
Actual m68k `sizeof(telnet_t)` is **42 bytes**. Option state grows in four-entry
chunks (two bytes/entry); suboption buffer grows 512/2048/8192/16384 bytes and
remains allocated until telnet_free. Optional event decoders can allocate more.
For five negotiated options plus a short suboption, state capacity alone is
42 + 16 + 512 = 570 bytes, excluding allocator overhead and event temporaries.
No library allocation would need to survive command exit or become Core idle RAM.

BusyBox's advertised 8.8 KB applet size is not a comparable 68000 measurement;
its libbb, sockets, poll and termios dependencies prevent treating that as a port
estimate. NCSA is a complete GUI application, not a separable engine benchmark.
No throughput or speed improvement has been measured for either alternative.

## Verification and recommendation boundary

All **11 libtelnet upstream host fixtures passed under ASan/UBSan**, including
RFC1143, TTYPE and malformed environment fixtures. This establishes those fixtures,
not comprehensive safety. No Snow port or replacement-client acceptance is claimed.
Reused existing host/Snow/physical VT evidence rather than rerunning unchanged
software; no production change required new emulator or hardware acceptance.
The current CLX and review recovery copy match SHA256
`8b13c31f4cc25672fc3ec2f876e1411895480c5837441c626e84e187df72ce16`.
Recovery source/binary and research build live under ignored work/telnet-review.
0.5 RC, installed software and all prior work are untouched; no push/release.

Next decision: if broader protocol/server work is approved, trial a pinned genuine
libtelnet adapter with no zlib, native allocation ownership, bounded suboption
policy, fatal-error mapping and explicit outgoing NVT conversion. Compare full
CLX size, connected peak/cleanup, fragmented/malformed negotiation, cancellation
and real-shell behaviour in Snow before physical acceptance. Do not introduce a
second protocol engine permanently or build a server as part of this review.
