# Rejected refresh-boundary batching experiment

18 November2026. Snow Plus/4MiB/System6.0.8 at fixed maximum acceleration.
Private diagnostic Core1819; baseline nano-profile1818 versus experimental1820,
installed through the normal repository updater. No SE access or push.

Research: pinned ncurses6.5 `lib_refresh.c` calls `doupdate_sp`; its public
`doupdate` delegates to the same implementation. `tty_update.c` flushes the update
before returning. Tested wrapping both entry points, suppressing only cooperative
paints during the refresh, preserving event handling and native update-event draws.
No ncurses source or renderer algorithm was changed; no new bitmap allocation.

Three matched warm runs per fixture are in same-name.json. Means (guest ticks):
empty buffer224.7→230.3; existing empty271.3→266.0; short8KiB443.3→426.3;
long8KiB339.7→356.0. Variations outside refresh account for much of these changes.
Short-file draws19–20→8, but rows remain77 and painting cost61–64→62–65 ticks.
Thus fewer draw calls did not mean less meaningful rendering work. No consistent
end-to-end improvement: **implementation removed**, including its experimental
runtime1.11 export. The following candidate retains runtime1.10.

Host tests passed for nesting, errors, nonlocal cancellation cleanup, actual
app_yield event handling while painting was deferred, and next-command recovery.
Existing300 renderer pixel cases,20000 inverse/cursor cases and200000 parser/dirty
state cases passed. The captured guest editor region matched pixel-for-pixel after
accounting for macOS active/inactive window-shadow offsets. This does not establish
physical hardware flicker acceptance.

The prototype source/builds and original packages remain recoverable in ignored
work/nano-profile2; they are not the production link path. This experiment does not
justify adding a new refresh API or permanent batching mechanism.
