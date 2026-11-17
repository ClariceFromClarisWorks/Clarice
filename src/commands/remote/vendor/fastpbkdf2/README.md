# fastpbkdf2

Joseph Birr-Pixton's CC0 implementation:
https://github.com/ctz/fastpbkdf2 (upstream.c and LICENSE retrieved 18 November 2026).

../../pbkdf2.inc retains the upstream generic HMAC/PBKDF2 implementation with a
thin adapter to Clarice's existing public-domain libutil SHA256. SHA1/SHA512 and
OpenSSL dependencies are omitted. Byte stores avoid unaligned 68000 accesses.
The iteration loop yields every 16 rounds for native UI/cancellation.

Only the optional telnetd module links this code. 1,000 rounds is a deliberate
legacy CPU compromise, not a contemporary offline password-hardening claim.
Use a unique strong password. Telnet carries it without transport encryption.
A five-second reconnect cooldown limits online guesses; the allowed-peer filter
is retained. Salt is unique time/tick/previous-salt derived data, not a claim of
cryptographically random key generation. No Unix file permissions are implied
on HFS: anyone able to read this volume can read the verifier.
