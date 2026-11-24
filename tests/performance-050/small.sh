mkdir /tmp/perf-small
touch /tmp/perf-small/a
ln -s /tmp/perf-small/a /tmp/perf-small/link
readlink /tmp/perf-small/link > /dev/null
hfsattrib /tmp/perf-small/a > /dev/null
basename /tmp/perf-small/a > /dev/null
dirname /tmp/perf-small/a > /dev/null
free > /dev/null
printenv PATH > /dev/null
mktemp /tmp/perf-small/file.XXXXXX > /dev/null
sleep 0
rm -r /tmp/perf-small
mkdir /tmp/perf-empty
rmdir /tmp/perf-empty
