HTTP_TEST_HOST=$1
echo start > /tmp/reuse-next/compare
/tmp/http-wget -O /tmp/wget-before http://$HTTP_TEST_HOST:8094/wget-before.clx
/tmp/http-wget -O /tmp/mktemp-before http://$HTTP_TEST_HOST:8094/mktemp-before.clx
for n in 1 2 3; do
for tool in /tmp/wget-before /tmp/http-wget; do
$tool -O /tmp/reuse-next/measure http://$HTTP_TEST_HOST:8094/length > /dev/null 2> /dev/null
echo "$tool:$?" >> /tmp/reuse-next/compare
status runtime >> /tmp/reuse-next/compare
rm /tmp/reuse-next/measure
done
for tool in /tmp/mktemp-before /tmp/mktemp-test; do
$tool /tmp/reuse-next/file.XXXXXX > /tmp/reuse-next/file-name
echo "$tool:$?" >> /tmp/reuse-next/compare
status runtime >> /tmp/reuse-next/compare
read name < /tmp/reuse-next/file-name
rm "$name"
done
done
/tmp/http-wget -O /tmp/reuse-next/recover http://$HTTP_TEST_HOST:8094/chunked
echo recovery:$? >> /tmp/reuse-next/compare
cmp /tmp/reuse-next/recover /tmp/reuse-next/length
echo integrity:$? >> /tmp/reuse-next/compare
ls -a /tmp/reuse-next > /tmp/reuse-next/listing
echo COMPARE_DONE
