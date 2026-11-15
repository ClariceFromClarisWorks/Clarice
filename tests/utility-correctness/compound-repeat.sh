export PATH=/mnt/Audit/tools:/bin:/usr/bin
cd /mnt/Audit/run
for repeat in 1 2 3; do
 sh /mnt/Audit/scopes.sh > /mnt/Audit/repeat-scopes.out
 echo scopes:$?
 sh /mnt/Audit/compound-doc.sh > /mnt/Audit/repeat-doc.out
 echo document:$?
 sh /mnt/Audit/records.sh > /mnt/Audit/repeat-records.out
 echo records:$?
 status runtime >> /mnt/Audit/repeat-memory.txt
done
sh /mnt/Audit/utility-files.sh > /mnt/Audit/utility-files.out 2> /mnt/Audit/utility-files.err
echo utilities:$?
echo READY-FOR-CANCELLATION
