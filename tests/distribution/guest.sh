# Run from the shell, no external tools required for setup itself.
export PATH=/bin:/usr/bin
status runtime > /mnt/Audit/d-start
appinstall setup /mnt/Audit/Utilities > /mnt/Audit/d-setup 2> /mnt/Audit/d-setup-errors
echo setup:$?
for name in ls df cat grep sed awk wc head tail sort find du file od cmp cksum free sleep printenv mktemp dd; do
 appinstall check /bin/$name > /tmp/checked 2> /tmp/check-error || echo check-failed:$name:$?
done
printf 'b\na\nb\n' | sort | uniq > /mnt/Audit/d-pipeline
awk 'BEGIN {print "awk works"}' > /mnt/Audit/d-awk
ls /bin > /mnt/Audit/d-bin
status runtime > /mnt/Audit/d-after
# Save current commands without relying on automatic installation at restart.
mv /bin/grep /tmp/original-grep
printf broken > /bin/grep
appinstall setup /mnt/Audit/Utilities > /tmp/kept
echo kept:$?
appinstall check /bin/grep > /tmp/check-out 2> /tmp/check-error
echo damaged-check:$?
appinstall repair grep /mnt/Audit/Utilities > /tmp/repaired 2> /tmp/repair-error
echo repair:$?
printf 'hello\n' | grep hello
appinstall recover grep > /tmp/recovered 2> /tmp/recovery-error
echo recovery:$?
cat /bin/grep
# Keep displaced valid version and original recovery evidence for host inspection.
appinstall repair grep /mnt/Audit/Utilities > /tmp/blocked 2> /tmp/blocked-error
echo blocked:$?
mv /bin/grep /tmp/recovered-damage
mv /bin/.clx-grep.new /tmp/repaired-grep
mv /tmp/original-grep /bin/grep
# An incomplete pre-existing stage must never be overwritten.
mv /bin/sed /tmp/original-sed
printf unfinished > /bin/.clx-sed.new
appinstall setup /mnt/Audit/Utilities > /tmp/partial 2> /tmp/partial-error
echo partial:$?
cat /bin/.clx-sed.new
mv /bin/.clx-sed.new /tmp/interrupted-stage
appinstall setup /mnt/Audit/Utilities > /tmp/resumed 2> /tmp/resumed-error
echo resumed:$?
# Real Macintosh heap pressure, in the isolated diagnostic core only.
mv /bin/awk /tmp/original-awk
appinstall test-reserve 290000
echo reserve:$?
status runtime > /mnt/Audit/d-low-before
appinstall setup /mnt/Audit/Utilities > /tmp/low-out 2> /mnt/Audit/d-low-error
echo low-setup:$?
appinstall test-reserve 0
status runtime > /mnt/Audit/d-low-after
appinstall setup /mnt/Audit/Utilities > /tmp/retry-out 2> /tmp/retry-error
echo retry:$?
awk 'BEGIN {print "awk recovered"}'
for n in 1 2 3; do appinstall setup /mnt/Audit/Utilities > /tmp/repeated; done
status runtime > /mnt/Audit/d-final
echo DISTRIBUTION_DONE
