# Shared libc retained storage — 16 November 2026

No runtime/library change made. Pinned Retro68 toolchain reports **newlib 4.6.0**.
The shared native funopen/fclose imports use the application's FILE pool, outside
the portable invocation-owned allocation counter.

`tests/utility-correctness/libc-pool.c` opens/closes 32 callback streams without
native file references, three times. Snow Plus/4 MB, build 1540:

| Pass | Before open | While open | After close | Prior FILE addresses reused |
|---|---:|---:|---:|---:|
| First | 408,964 B | 405,888 B | 406,784 B | — |
| Repeat 1 | 406,784 B | 405,888 B | 406,784 B | 32/32 |
| Repeat 2 | 406,784 B | 405,888 B | 406,784 B | 32/32 |

`sizeof(FILE)` is 104 bytes. Retained increment is **2,180 B (2.13 KiB)**;
896 B returns after each close cycle. This is reusable stdio capacity, not an
accumulating invocation leak. It is **not generally reclaimable application-heap
capacity**: closed slots remain allocated to libc rather than becoming purgeable
Macintosh Handles or arbitrary malloc space. Earlier ~2.10 KiB observation used a
different opening sequence; do not treat the small difference as a leak trend.

Source inspection: [newlib findfp.c](https://github.com/mirror/newlib-cygwin/blob/master/newlib/libc/stdio/findfp.c)
searches free records and grows a shared glue list; [fclose.c](https://github.com/mirror/newlib-cygwin/blob/master/newlib/libc/stdio/fclose.c)
releases stream buffers and marks records free for reuse. There is no public trim
operation in this path. [_reclaim_reent](https://github.com/mirror/newlib-cygwin/blob/master/newlib/libc/reent/reent.c)
is not a safe per-command reset of the active shared libc; its global stream
storage is shared by Clarice and ports. Source inspection corroborates the observed
reuse; it is not a physical-hardware measurement or proof of all libc allocations.

Recommendation: retain the modest high-water pool. Manually unlinking FILE records
would depend on internal libc state, locking and live-stream ownership, with no
measured workload benefit here. Do not call it OS-reclaimable memory, and do not
claim zero libc retention. No new cache or allocator framework is proposed.

Raw evidence: workflows/libc-pool.out. Host stream tests and prior guest runtime
fault tests independently cover descriptor closure and invocation allocations.
