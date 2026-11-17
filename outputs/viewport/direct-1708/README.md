# Direct composition 1708 — Snow evidence

Fixed accelerated Plus / 4 MB / System 6.0.8 fixture. Three trials per variant;
24 completed large-jump redraws, guest ticks. `new=0` is glyph-plane baseline;
light `new=1` is direct presentation; dark `new=2` uses RAM inversion plus srcCopy.
Dark `new=1` deliberately measures the slower notSrcCopy control, not the selected path.

Medians: dense light 479→425 (11.3%), dark 544→465 (14.5%); sparse listing
249→249 and 290→291. Phase counters overlap. This is redraw time, not ls duration.
35 pixel comparisons and 149 scrollbar comparisons pass; 22 scrolling bursts /
20 views match. Heap pressure probe passes reclaim, locked refusal and exhaustion
recovery with zero owned allocations. Free heap +12 B; largest block −1,864 B.
No unchanged-fragmentation or full application-pressure claim.

Source/build hashes identify the isolated scratch build. Physical acceptance is
separate and pending. No release-candidate source or package was changed.
