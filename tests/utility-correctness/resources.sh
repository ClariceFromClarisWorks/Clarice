export PATH=/bin:/usr/bin
prefs paging off
status runtime
/mnt/Audit/cp -Rp /mnt/Audit/deep /mnt/Audit/copied
echo COPY:$?
/mnt/Audit/find /mnt/Audit/copied > /mnt/Audit/copied-list
/mnt/Audit/rm -r /mnt/Audit/copied
echo REMOVE:$?
/mnt/Audit/find /mnt/Audit/copied
echo ABSENT:$?
/mnt/Audit/fault 12 0 cp -R /mnt/Audit/deep /mnt/Audit/partial
echo COPY_OOM:$?
/mnt/Audit/rm -rf /mnt/Audit/partial
echo CLEAN:$?
/mnt/Audit/fault 0 12 cp -R /mnt/Audit/deep /mnt/Audit/partial
echo COPY_CANCEL:$?
/mnt/Audit/rm -rf /mnt/Audit/partial
/mnt/Audit/cp -R /mnt/Audit/deep /mnt/Audit/partial
/mnt/Audit/fault 12 0 rm -r /mnt/Audit/partial
echo REMOVE_OOM:$?
/mnt/Audit/fault 0 12 rm -r /mnt/Audit/partial
echo REMOVE_CANCEL:$?
/mnt/Audit/rm -rf /mnt/Audit/partial
echo RECOVER:$?
/mnt/Audit/cp -p /mnt/Audit/fork-source /mnt/Audit/fork-copy
echo FORKS:$?
/mnt/Audit/streams /mnt/Audit/stream-input 16
echo STREAMS1:$?
status runtime
/mnt/Audit/streams /mnt/Audit/stream-input 16
echo STREAMS2:$?
status runtime
/mnt/Audit/streams /mnt/Audit/stream-input 16
echo STREAMS3:$?
status runtime
