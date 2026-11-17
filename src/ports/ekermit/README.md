# E-Kermit adapter

Upstream: [E-Kermit 1.8](https://www.kermitproject.org/ek.html), Frank da Cruz,
Copyright 1995, 2021 Trustees of Columbia University. Revised 3-clause BSD;
see `upstream/COPYING`. The five upstream files are unmodified. `kermit.c` is
our separately maintained adaptation; compare it directly with upstream/kermit.c.
Source archive: https://www.kermitproject.org/ftp/kermit/archives/ek18.tar
SHA-256: 59c54f3fee05797ae2c3fbb9905cd518938bce86db83675a377532289ea2da1b.

Build: short packets, one window, CRC-16 forced (upstream check type 5), no
attributes, scanning or text conversion. Both endpoints must support forced
CRC-16. Our supplied host peer uses the same engine. No general Kermit server,
batch transfers, resume or remote management. Host peer takes a local device
explicitly and is a one-shot transfer process, not a shell or network service.

Local patches bound decoded filenames and prefix operands, validate short-packet
lengths before indexing, cap negotiated packet sizes, stop decoding immediately
on write failure, preserve tail-write failures, and correctly return success for
an empty-file EOF. The selected build excludes upstream long-packet and attribute
parsers; it is not a claim that all upstream configurations have been audited.

Adapter: 512-byte file buffers, 256-byte packet input queue, native 4096-byte
serial receive buffer, 64-byte filename bound. All guest BSS belongs to the loaded
command and is released on return. Existing Serial-1 driver setup/async write and
safe receive-buffer restoration are reused as the native transport pattern.
No terminal rendering or new runtime service is added. Exclusive sequential
port ownership: leave `serial`, run `kermit`, then reconnect `serial`.

Receive always requires an explicit new destination; wire filenames are checked
but never used as filesystem paths. One file per invocation; no overwrite or
symlink traversal at the destination. An owned same-directory temporary is closed
and renamed only at a successful file EOF, before acknowledging that EOF. An
interruption before that point removes the temporary. Loss of the final transaction
ACK can leave a complete committed file with a failed overall status; it is never
a partial successful file. Retrying requires a new destination or explicit removal.
Ordinary transfers carry data forks only. Package complete Macintosh files in
MacBinary before transfer; this command does not create or unpack containers.

Cancellation and native I/O errors terminate the invocation; pending async writes
are cancelled before releasing their storage. Five consecutive two-second read
timeouts and a ten-minute overall deadline bound waits. Three-wire serial has no
reliable remote-disconnect signal. The peer can time out after local cancellation.

14 November hardware follow-up: fixed the short-packet off-by-one limit. LEN
counts sequence/type/data/checksum but excludes LEN itself; a maximum LEN=94
requires 95 buffered bytes (plus NUL). Both packet collection and validation
now permit that within the existing P_PKTLEN+8 allocation. Deterministic
maximum-short regression exercises real bidirectional adapter/engine transport.
The previously passing byte-pattern fixtures did not exercise this boundary.
