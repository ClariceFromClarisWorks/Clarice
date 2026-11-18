# Complete utility deployment, 18 November 2026

This is a supervised deployment of the existing compatible build1713 utility
collection, using the existing appinstall replacement/recovery operation. It is
**not acceptance of the unfinished coordinated Core/update transaction**.
The accepted diagnostic Core1713 already runs on both guests and is retained;
its binary is not identical to the normal Core in the bundle. No Core handoff,
relaunch, release, or new production functionality is claimed here.

The sealed clean build contains47 standard command names /32 distinct CLX images.
Bundle identity: 2b8d4b5597bb4a6055b45c859d7e2019f544b3ef1e74d779b98ccacced1db6a9.
All payloads are transferred in one1,351,680-byte USTAR file. A temporary12,020-byte
CLX fixture (`tests/release/native-stage-fixture.c`, built by
`sh tests/release/build-native-stage.sh`) reuses the restricted release reader and native HFS primitives to
extract into/tmp, refusing existing files. Payload length/CRC and archive structure
are checked. appinstall check then applies the actual full production loader
validation to every staged utility before installation. The fixture is not an
additional production installer; extraction was restricted to the disposable /tmp staging directory during these tests.

Snow Plus/4MB/System6.0.8, accelerated~15x:
- Whole bundle extraction and all47 full CLX checks passed.
- Existing recovery-path preflight passed.
- appinstall replaced all47 commands, retaining previous files.
- Read-back of every installed utility matches manifest length and CRC.
- Existing extraction destination rejected with error-48/status1, no overwrite.
- ls/wc pipeline, cksum(test)=3076352578/4 and grep passed.
- Following grep, tracked remaining allocation0; freeheap347720, largest345448.
- Optional nano remains outside the standard inventory (48 visible/bin entries).

The MacBook NAT guest connects to the host's LAN address; attempting the emulator
router/another NAT subnet did not reach the host HTTP listener. Fast-forward also
accelerates guest key repeat: a1ms key-down helper avoided duplicated automation
characters. These are test-harness observations, not production changes.

Physical SE68000/4MB/System6.0.8: all47 staged CLX checks passed, six old
standard-tool recovery files moved without deletion into
/etc/clarice/recovery-pre1713, then all47 replacements/read-back checks passed.
The existing per-file installer retained each replaced file in /bin/.clx-NAME.old.
The two optional nano recovery files were left untouched. Configuration/profile
cksum before/after comparisons passed. grep, sed, awk, cmp, cksum, FTP/Telnet help
and native ipconfig passed; final markers INSTALL-47-PASS, SETTINGS-PASS,
SMOKE-PASS and DEPLOYMENT-1713-COMPLETE are captured. This is installation and
smoke acceptance, not a repeated networking-protocol/hardware performance suite.
Following command completion: freeheap326642, largest147486, tracked remaining0;
no new repeated-run/fragmentation stability claim. Timings were not measured
reliably; completion was established from the closed log, not a visible prompt.
A6.55KiB unrolled read-only preflight delayed foreground processing enough to lose
SEControl. Webcam showed the command still pending, no error dialog. Serial57600
8N1 opened but received no bytes before or after Ctrl+C/Return: no SE maintenance
receiver exists. The helper subsequently reconnected and the script reported
BLOCKED-ls-old. Cause of the long pause is not established. Compact loops replace
the unrolled checks for the remaining supervised deployment.

Known limitation: per-command appinstall recovery is used here. The generic
collection transaction, durable native recovery/startup gate and normal HTTP/offline
integration remain unfinished. This one-off verified deployment does not make
future upgrades automatically coherent or power-loss atomic. All production
Core/utility compatibility and recovery policies remain unchanged.

Cleanup: on the physical SE, the installation log was copied and byte-compared
to /etc/clarice/deployment1713.log before removing /tmp/full1713. CLEANUP-PASS
was observed. Recovery copies were retained.
The same log preservation, comparison and staging cleanup passed in Snow.
