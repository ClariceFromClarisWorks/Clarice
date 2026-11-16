echo CANCEL:$? > /mnt/Audit/practical/heap-cancel-status
awk 'BEGIN { for(i=0;i<500;i++)a[i]=i; print a[499] }'
echo RECOVERY:$?
status runtime
echo STDOUT_RESTORED
echo STDERR_RESTORED >&2
cd /
/mnt/Audit/finish
