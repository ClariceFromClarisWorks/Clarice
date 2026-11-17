# 1709 — remove duplicate row validation

First row retains checked font initialisation; subsequent nonzero-phase rows use
existing compositor after the whole viewport has already passed validation.
No event yield occurs between validation and composition. No added buffer.

Snow medians (three trials, 24 large-jump draws): light baseline475/direct389;
dark baseline537/direct429 guest ticks. Previous1708 direct425/465. Composition
falls from263–264 to227–230 ticks per23 staged frames. Separate builds show a
small baseline drift, so the precise incremental percentage is approximate.
Sparse listing247→250 and290→291; one to three ticks remains minor gate overhead.
35 pixel checks,22 scrolling bursts/20 views,149 scrollbar checks pass. Native
pressure tests pass with owned0/free heap+12B/largestblock−1864B. MacBinary size
unchanged288384B; initial nativefreeheap64B lower than1708, consistent with code
layout rather than another permanent bitmap. Physical acceptance is pending.
