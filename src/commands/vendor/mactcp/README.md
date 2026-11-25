MIT network adapter from Macinclaude, Bart Decrem, commit
f4b06160b186c95f0ebc234214fe916ae2b463b9 (net/nettcp.c and net/nettcp.h).
Reuses the separately credited adapter qualified by SEControl; no GPL mouse
code or SEControl runtime dependency. Local adaptations: pinned Multiversal
headers, initialized parameter blocks, five-second driver timeouts, negative
OSErr propagation and peer-close polling. Connect/send/receive/close now use
asynchronous PBs with the existing cooperative event wait. Foreground teardown
retains PB/WDS/caller buffers until completion and retries failed release before
unloading; Telnetd retains its existing registered-service ownership path.
A permanently broken driver can prevent safe return; no orphan-I/O service or
new resident storage is introduced. See tests/network-4b.
