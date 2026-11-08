echo SED-START
status runtime
sed 's/alpha/A/g' /tmp/grep-input
echo substitution:$?
cat /tmp/grep-input | sed -n '2,4p' | grep -E 'Beta|123'
echo pipeline:$?
sed -n '1,2w /tmp/sed-written' /tmp/grep-input
cat /tmp/sed-written
echo write:$?
sed 's/[/' /tmp/grep-input 2> /tmp/sed-error
echo invalid:$?
cat /tmp/sed-error
for n in 1 2 3 4 5; do sed -n '1p' /tmp/grep-input > /tmp/sed-loop || echo FAIL; done
status runtime
echo SED-END
