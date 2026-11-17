cd /mnt/Audit
echo ELASTIC_START
echo CASE small
/mnt/Audit/sort.clx small > small.out 2> small.err
echo STATUS $?
status runtime
echo CASE long
/mnt/Audit/sort.clx long > long.out 2> long.err
echo STATUS $?
status runtime
echo CASE unique
/mnt/Audit/sort.clx -u long > unique.out 2> unique.err
echo STATUS $?
status runtime
echo CASE record
/mnt/Audit/sort.clx record > record.out 2> record.err
echo STATUS $?
status runtime
echo CASE length
/mnt/Audit/awk.clx '{print length($0)}' record > length.out 2> length.err
echo STATUS $?
status runtime
echo CASE fields
/mnt/Audit/awk.clx '{print NF,length($0),length($NF)}' fields > fields.out 2> fields.err
echo STATUS $?
status runtime
echo CASE oom-sort
/mnt/Audit/sort.clx oom > oom-sort.out 2> oom-sort.err
echo STATUS $?
status runtime
echo CASE oom-awk
/mnt/Audit/awk.clx '{print length($0)}' oom > oom-awk.out 2> oom-awk.err
echo STATUS $?
status runtime
echo CASE recovery-sort
/mnt/Audit/sort.clx small > recovery-sort.out 2> recovery-sort.err
echo STATUS $?
status runtime
echo CASE recovery-awk
/mnt/Audit/awk.clx '{print length($0)}' small > recovery-awk.out 2> recovery-awk.err
echo STATUS $?
status runtime
echo ELASTIC_DONE
