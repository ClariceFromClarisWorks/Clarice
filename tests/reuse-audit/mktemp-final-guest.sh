HTTP_TEST_HOST=$1
/tmp/http-wget -O /tmp/mktemp-final http://$HTTP_TEST_HOST:8094/mktemp-after.clx
mkdir /tmp/reuse-final
for n in 1 2 3; do
for tool in /tmp/mktemp-before /tmp/mktemp-final; do
$tool /tmp/reuse-final/file.XXXXXX > /tmp/reuse-final/name
echo "$tool:$?" >> /tmp/reuse-final/result
status runtime >> /tmp/reuse-final/result
read name < /tmp/reuse-final/name
rm "$name"
done
/tmp/mktemp-final -d /tmp/reuse-final/dir.XXXXXX > /tmp/reuse-final/name
echo dir:$? >> /tmp/reuse-final/result
status runtime >> /tmp/reuse-final/result
read name < /tmp/reuse-final/name
test -d "$name"
echo kind:$? >> /tmp/reuse-final/result
rmdir "$name"
done
/tmp/mktemp-final -d /tmp/reuse-final/fail.XXXXXX > /dev/full
echo output-error:$? >> /tmp/reuse-final/result
/tmp/mktemp-final -dq /mnt/NoSuchVolume/d.XXXXXX > /tmp/reuse-final/name
echo missing:$? >> /tmp/reuse-final/result
ls -a /tmp/reuse-final > /tmp/reuse-final/listing
echo FINAL_DONE
