echo CANCEL:$? > /mnt/Audit/cancel-status
printf 'stdout-restored\n'
printf 'stderr-restored\n' >&2
printf 'clean\n' > /mnt/Audit/cancel-output
cat /mnt/Audit/cancel-output
status runtime > /mnt/Audit/cancel-memory.txt
/mnt/Audit/libc-pool > /mnt/Audit/libc-pool-repeat.out
cd /
/mnt/Audit/finish
echo FINISH:$?
