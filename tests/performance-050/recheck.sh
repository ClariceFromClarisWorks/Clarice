echo ACCEPT_BEGIN
prefs paging off
/mnt/Audit/before/find /mnt/Audit/tree > /mnt/Audit/find-before
find /mnt/Audit/tree > /mnt/Audit/find-after
cmp /mnt/Audit/find-before /mnt/Audit/find-after
echo find-output:$?
/mnt/Audit/before/du -a /mnt/Audit/tree > /mnt/Audit/du-before
du -a /mnt/Audit/tree > /mnt/Audit/du-after
cmp /mnt/Audit/du-before /mnt/Audit/du-after
echo du-output:$?
cd /mnt/Audit/tree
/mnt/Audit/before/find . > /mnt/Audit/rel-before
find . > /mnt/Audit/rel-after
cmp /mnt/Audit/rel-before /mnt/Audit/rel-after
echo relative:$?
cd /
find /mnt/Absent-Volume > /dev/null
echo missing:$?
cp -R /mnt/Audit/tree /tmp/perf-tree
/mnt/Audit/before/du -a /tmp/perf-tree > /mnt/Audit/boot-before
du -a /tmp/perf-tree > /mnt/Audit/boot-after
cmp /mnt/Audit/boot-before /mnt/Audit/boot-after
echo boot:$?
rm -r /tmp/perf-tree
wc /mnt/Audit/utf > /mnt/Audit/wc-utf
tr a-z A-Z < /mnt/Audit/utf > /mnt/Audit/tr-utf
wc /mnt/Audit/binary > /mnt/Audit/wc-binary
tr a-z A-Z < /mnt/Audit/binary > /mnt/Audit/tr-binary
status runtime
echo ACCEPT_DONE
