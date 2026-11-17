export PATH=/bin:/usr/bin
/mnt/Audit/stream-fault /mnt/Audit/stream-input 16 sequential 5 0
echo STREAM_OOM:$?
status runtime
/mnt/Audit/stream-fault /mnt/Audit/stream-input 16 sequential 0 10
echo STREAM_CANCEL:$?
status runtime
/mnt/Audit/streams /mnt/Audit/stream-input 16 sequential
echo STREAM_RECOVERY:$?
status runtime
/mnt/Audit/awk -f /mnt/Audit/many.awk
echo AWK:$?
status runtime
/mnt/Audit/awk -f /mnt/Audit/many.awk
echo AWK_REPEAT:$?
status runtime
