MIT network adapter from Macinclaude, Bart Decrem, commit
f4b06160b186c95f0ebc234214fe916ae2b463b9 (net/nettcp.c and net/nettcp.h).
Reuses the separately credited adapter qualified by SEControl; no GPL mouse
code or SEControl runtime dependency. Local adaptations: pinned Multiversal
headers, initialized parameter blocks, five-second driver timeouts, negative
OSErr propagation, peer-close polling, retain receive memory on failed release.
