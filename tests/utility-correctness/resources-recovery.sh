export PATH=/bin:/usr/bin
/mnt/Audit/fault 1000 0 cp /mnt/Audit/stream-input /mnt/Audit/fork-copy
echo WRITE_FAIL:$?
/mnt/Audit/fault 0 2 cp /mnt/Audit/stream-input /mnt/Audit/fork-copy
echo WRITE_CANCEL:$?
/mnt/Audit/streams /mnt/Audit/stream-input 16
echo CAPACITY:$?
status runtime
/mnt/Audit/streams /mnt/Audit/stream-input 16 sequential
echo SEQUENTIAL1:$?
status runtime
/mnt/Audit/streams /mnt/Audit/stream-input 16 sequential
echo SEQUENTIAL2:$?
status runtime
/mnt/Audit/streams /mnt/Audit/stream-input 16 sequential
echo SEQUENTIAL3:$?
status runtime
