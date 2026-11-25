case b in [a\-c]) echo FAIL-range; exit 1;; *) echo PASS-case;; esac
case - in [a\-c]) echo PASS-hyphen;; *) exit 1;; esac
x=bword; test "${x#[a\-c]}" = bword || exit 1
x=wordb; test "${x%[a\-c]}" = wordb || exit 1
set -- x '~'; set -- $@; test "$2" = '~' || exit 1
printf 'at=<%s>\n' "$@"
awk 'BEGIN {print ENVIRON["HOME"]}' > /tmp/closure-home
read home < /tmp/closure-home; test "$home" = /home || exit 1
HOME=/custom awk 'BEGIN {print ENVIRON["HOME"]}' > /tmp/closure-home
read home < /tmp/closure-home; test "$home" = /custom || exit 1
env -i /bin/awk 'BEGIN {print ("HOME" in ENVIRON)}' > /tmp/closure-home
read home < /tmp/closure-home; test "$home" = 0 || exit 1
awk 'BEGIN {print ENVIRON["HOME"]}' > /tmp/closure-home
read home < /tmp/closure-home; test "$home" = /home || exit 1
echo PASS-home
mkdir -p /tmp/closure-glob
for name in a b c -; do touch /tmp/closure-glob/$name; done
cd /tmp/closure-glob
printf '<%s>\n' [a\-c] > /tmp/closure-glob-result
printf '<->\n<a>\n<c>\n' > /tmp/closure-expected
cmp /tmp/closure-glob-result /tmp/closure-expected || exit 1
printf '<%s>\n' [a-c] > /tmp/closure-glob-result
printf '<a>\n<b>\n<c>\n' > /tmp/closure-expected
cmp /tmp/closure-glob-result /tmp/closure-expected || exit 1
echo PASS-glob
cd /
appinstall check /bin/closure-entry || exit 1
cp /etc/clarice/clx-closure-entry.ok /tmp/closure-receipt || exit 1
/mnt/Sys6Unix/Closure/pressure.clx /bin/closure-entry
r=$?; echo refused=$r; test "$r" = 126 || exit 1
cmp /etc/clarice/clx-closure-entry.ok /tmp/closure-receipt || exit 1
echo PASS-receipt
/bin/closure-entry || exit 1
status runtime
for i in 1 2 3 4 5; do /bin/closure-entry || exit 1; status runtime; done
! false | true; test "$?" = 1 || exit 1
! true | false; test "$?" = 0 || exit 1
echo CLOSURE-LOCAL-DONE
