for repeat in 0 1 2; do
 echo CASE find-before "$repeat"
 time /mnt/Audit/before/find /mnt/Audit/hundred > /dev/null
 echo STATUS $?
 status runtime
 echo CASE find-after "$repeat"
 time find /mnt/Audit/hundred > /dev/null
 echo STATUS $?
 status runtime
 echo CASE du-before "$repeat"
 time /mnt/Audit/before/du /mnt/Audit/hundred > /dev/null
 echo STATUS $?
 status runtime
 echo CASE du-after "$repeat"
 time du /mnt/Audit/hundred > /dev/null
 echo STATUS $?
 status runtime
 done
 echo AUDIT_DONE
