TEST_HOST=${1:?pass the trusted fixture host address}
status runtime
for n in ipconfig ping wget ftp telnet; do
 appinstall check /bin/$n
 echo check-$n:$?
 $n --help > /tmp/help-$n
 echo help-$n:$?
done
ls /bin | wc -w
printf 'beta\nalpha\nbeta\n' | sort | uniq > /tmp/words
grep alpha /tmp/words
awk 'BEGIN {print "AWK_READY"}'
mktemp -d /tmp/rc-XXXXXX
ipconfig
echo ip:$?
for i in 4 5 6; do
 ftp -P 2121 get ${TEST_HOST} binary.dat /tmp/ftp-$i
 echo get-$i:$?
 ftp -P 2121 put ${TEST_HOST} /tmp/ftp-$i uploaded-$i.dat
 echo put-$i:$?
 status runtime
done
ftp -P 2121 get ${TEST_HOST} binary.dat /tmp/ftp-4
echo exists:$?
wget -O /tmp/http-final http://${TEST_HOST}:8094/chunk
echo http:$?
cmp /tmp/http-final /tmp/ftp-4
echo bytes:$?
status runtime
echo RC_DONE
