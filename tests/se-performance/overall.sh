# Run locally on the SE after recording installed versions and disabling services.
# Fixture preparation is excluded from timings. Each first case is first in this
# session, not a cold boot. Preserve output and record webcam timestamps separately.
mkdir /tmp/seperf1818
if test $? -ne 0; then
 echo 'Refusing existing or unavailable fixture directory'
 exit 2
fi
dd if=/dev/zero of=/tmp/seperf1818/input bs=1024 count=64
if test $? -ne 0; then exit 2; fi
printf 'alpha beta\nsecond line\n' > /tmp/seperf1818/text
status runtime
for repeat in 0 1 2 3; do
 echo SAMPLE "$repeat" builtin
 time printf ''
 echo STATUS $?
 status runtime
 echo SAMPLE "$repeat" external
 time wc -c /tmp/seperf1818/text
 echo STATUS $?
 status runtime
 echo SAMPLE "$repeat" read64k
 time wc -c /tmp/seperf1818/input
 echo STATUS $?
 status runtime
 echo SAMPLE "$repeat" copy64k
 time cp /tmp/seperf1818/input /tmp/seperf1818/copy
 echo STATUS $?
 status runtime
 cmp /tmp/seperf1818/input /tmp/seperf1818/copy
 echo VERIFY $?
 echo SAMPLE "$repeat" pipeline
 time sh -c 'cat /tmp/seperf1818/text | grep alpha | wc -l'
 echo STATUS $?
 status runtime
 echo SAMPLE "$repeat" catalog
 time ls -l /bin > /dev/null
 echo STATUS $?
 status runtime
done
echo SE_PERFORMANCE_DONE
# Retain fixtures for exact readback; no automatic cleanup of unexpected files.
