echo FINAL-BEGIN
test ! -f /etc/clarice/update.incomplete || exit 1
appinstall check /bin/clarice || exit 2
appinstall check /bin/ls || exit 3
appinstall check /bin/awk || exit 4
printf 'three\ntwo\none\n' | sort | wc -l || exit 5
awk 'BEGIN {print "AWK-OK"}' || exit 6
grep 'keep me' /etc/clarice/repo-user.txt || exit 7
cat /etc/clarice/installed.catalog || exit 8
clarice update -y --source http://192.168.1.112:8083/full/index.txt || exit 9
status runtime
clarice update -y --source http://192.168.1.112:8083/full/index.txt || exit 10
status runtime
ls -la /mnt/GlyphFull || exit 11
echo FINAL-PASS
