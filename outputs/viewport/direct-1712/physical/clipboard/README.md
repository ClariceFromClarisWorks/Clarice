# Physical SE clipboard acceptance — build 2026111712

17 November 2026: stock SE/68000, 4 MB, System 6.0.8. Actual SEControl mouse
selection and native keyboard/menu events, not direct calls into clipboard code.

- Selected the visible `COPYCHECK-SE` text; Command-C then Command-V placed those
  exact selected bytes in the live command line. No Return or execution occurred.
- Selected all of `COPYCHECK-SE6`; Edit → Copy, Edit → Paste produced that exact
  text in the command line. Both menu actions exercised by dragging from Edit.
- Produced 40 numbered lines, scrolled up with the actual scrollbar arrow,
  selected historical `COPY-14`, then Command-C/V. Paste returned to the live
  bottom and inserted `COPY-14`, without executing it.
- Cleared each test line with Ctrl-U. Subsequent commands ran normally.
- Existing host paste/selection sanitizer tests pass (capacity, CR/LF, invalid
  controls, allocation failures, selection direction/anchor crossing).

The reported PowerBook failure is NOT reproduced in these System 6 workflows.
No clipboard implementation changes. PowerBook/System 7 remains unavailable and
untested; these results do not diagnose its failure. No cross-application Scrap
Manager test or new large-paste hardware test is claimed. Screenshots alongside
this file show selections and resulting input.
