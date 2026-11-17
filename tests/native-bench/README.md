# Isolated native launch probe

Run `sh tests/native-bench/build.sh` from the repository root using the existing
pinned Retro68 image. Outputs stay in `work/native-bench`, outside source control.
`nb.clx` is the common driver. `nbi.clx` embeds the native worker package, uses the
existing production MacBinary parser/extractor and creates only `/tmp/NBWorker`.
It refuses an existing destination; it is not a general unpacker or updater.

In a disposable guest:

```
/tmp/nbi
echo install=$?
appinstall replace /tmp/nb nb
nb clx
status runtime
nb native /tmp/NBWorker
status runtime
```

Repeat the last four commands three times. Expect argument `alpha beta`, stderr
`probe stderr`, command status 7 and no retained invocation allocations. Both
routes use the same installed-fast CLX driver; the native route additionally
sublaunches a separately linked APPL with a 160 KiB partition. A fresh `NB Request`
file beside the worker transports a bounded result. Normal completion deletes it;
timeout/cancellation leaves it for inspection and refuses a subsequent overwrite.
Only remove it explicitly after confirming the worker is no longer running.

No production process runtime, live streams or worker cancellation protocol is
implemented. Never treat the driver's timeout as proof the child has exited.
See `outputs/native-bench/RESULTS.md` for actual measurements and limitations.
