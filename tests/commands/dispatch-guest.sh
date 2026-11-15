# Disposable /tmp fixtures only. Run on the normal guest build.
ls -1P /mnt/Sys6Unix/Bench/n10 > /tmp/dispatch-names
if test "$?" != 0; then echo FAIL-list; exit 1; fi
/bin/cat /tmp/dispatch-names > /tmp/dispatch-copy
if test "$?" != 0; then echo FAIL-absolute; exit 1; fi
mkdir /tmp/dispatch-bin
ln -s /bin/ls /tmp/dispatch-bin/ls
/tmp/dispatch-bin/ls -1P /mnt/Sys6Unix/Bench/n10 > /tmp/dispatch-linked
if test "$?" != 0; then echo FAIL-link; exit 1; fi
printf 'echo script-ok\n' > /tmp/dispatch-script.sh
if test "$(/tmp/dispatch-script.sh)" != script-ok; then echo FAIL-script; exit 1; fi
printf '#!/bin/sh\necho shebang-ok\n' > /tmp/dispatch-script
if test "$(/tmp/dispatch-script)" != shebang-ok; then echo FAIL-shebang; exit 1; fi
printf 'not executable\n' > /tmp/dispatch-bad
/tmp/dispatch-bad
if test "$?" != 126; then echo FAIL-reject; exit 1; fi
/tmp/dispatch-missing
if test "$?" != 127; then echo FAIL-missing; exit 1; fi
export PATH=/tmp:/bin:/usr/bin
dispatch-script
if test "$?" != 0; then echo FAIL-path; exit 1; fi
export PATH=/bin:/usr/bin
printf 'CLX1truncated' > /tmp/dispatch-corrupt
/tmp/dispatch-corrupt
if test "$?" != 126; then echo FAIL-corrupt; exit 1; fi
rm -f /tmp/dispatch-names /tmp/dispatch-copy /tmp/dispatch-link /tmp/dispatch-bin/ls /tmp/dispatch-linked /tmp/dispatch-script.sh /tmp/dispatch-script /tmp/dispatch-bad /tmp/dispatch-corrupt
rmdir /tmp/dispatch-bin
echo DISPATCH-GUEST-PASS
