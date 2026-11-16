echo FINAL_BEGIN
for repeat in 0 1 2; do
echo CASE wc-large "$repeat"
time wc /mnt/Audit/large > /dev/null
echo STATUS $?
status runtime
echo CASE tr-large "$repeat"
time tr a-z A-Z < /mnt/Audit/large > /dev/null
echo STATUS $?
status runtime
done
for batch in 0 1; do
for repeat in 0 1 2 3 4; do
wc /mnt/Audit/small > /dev/null
tr a-z A-Z < /mnt/Audit/small > /dev/null
find /mnt/Audit/ten > /dev/null
done
echo HEAP_BATCH "$batch"
status runtime
done
echo AUDIT_DONE
