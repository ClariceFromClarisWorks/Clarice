# Live bulk-output experiment1711 (Snow only)

Extends the1710 complete-screen path to finished, noninteractive output batches;
partial rows, canonical reads, search, paging, selection and overlapping scrolling
retain the existing renderer. No added rendering allocation.

293 pixel checks pass in the Plus/4MB/System6.0.8 fixture at maximum acceleration.
First timings include fixture generation inside the timed interval and are retained
for transparency, not used as the final terminal-performance estimate. Refined1712
moves formatting outside the interval and records ingestion/painting separately;
renderer source is unchanged. No1711 physical deployment or RC promotion.
