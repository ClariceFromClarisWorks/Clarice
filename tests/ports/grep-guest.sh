echo GREP-START
status runtime
/bin/grep -n alpha /tmp/grep-input
echo match:$?
/bin/grep -E '^(alpha|Beta)$' /tmp/grep-input
echo regex:$?
/bin/grep -F missing /tmp/grep-input
echo nomatch:$?
/bin/grep -E '[' /tmp/grep-input 2> /tmp/grep-error
echo invalid:$?
cat /tmp/grep-error
cat /tmp/grep-input | grep -i beta | cat
echo pipeline:$?
grep -c alpha < /tmp/grep-input > /tmp/grep-count
cat /tmp/grep-count
echo redirected:$?
grep alpha /tmp/no-such-file 2> /tmp/grep-error
echo missing:$?
cat /tmp/grep-error
grep -q alpha /tmp/grep-input
echo quiet:$?
grep alpha /tmp/grep-input 1>&-
echo closed:$?
grep -F a /tmp/grep-long 2> /tmp/grep-error
echo long:$?
cat /tmp/grep-error
for n in 1 2 3 4 5 6 7 8 9 10; do grep -q alpha /tmp/grep-input; done
status runtime
echo GREP-END
