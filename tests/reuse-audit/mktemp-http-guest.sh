HTTP_TEST_HOST=$1
mkdir /tmp/reuse-next
/tmp/mktemp-test -d /tmp/reuse-next/dir.XXXXXX > /tmp/reuse-next/dir-name
echo $? > /tmp/reuse-next/status
read directory < /tmp/reuse-next/dir-name
test -d "$directory"
echo $? >> /tmp/reuse-next/status
echo preserved > "$directory/owned"
/tmp/mktemp-test -dq /mnt/Missing/no.XXXXXX > /tmp/reuse-next/no-name
echo $? >> /tmp/reuse-next/status
/tmp/mktemp-test -d /tmp/reuse-next/another.XXXXXX > /dev/full
echo $? >> /tmp/reuse-next/status
for i in 1 2 3; do
/tmp/mktemp-test -d /tmp/reuse-next/repeat.XXXXXX > /tmp/reuse-next/repeat-name
echo $? >> /tmp/reuse-next/status
status runtime >> /tmp/reuse-next/memory
read directory < /tmp/reuse-next/repeat-name
rmdir "$directory"
done
for path in length chunked truncated malformed ambiguous 404; do
/tmp/http-wget -O /tmp/reuse-next/$path http://$HTTP_TEST_HOST:8094/$path
echo "$path:$?" >> /tmp/reuse-next/http-status
status runtime >> /tmp/reuse-next/http-memory
done
cmp /tmp/reuse-next/length /tmp/reuse-next/chunked
echo cmp:$? >> /tmp/reuse-next/http-status
for i in 1 2 3; do
/tmp/http-wget -O /tmp/reuse-next/repeat-$i http://$HTTP_TEST_HOST:8094/chunked
echo "$i:$?" >> /tmp/reuse-next/http-status
status runtime >> /tmp/reuse-next/http-memory
done
echo NEXT_DONE
