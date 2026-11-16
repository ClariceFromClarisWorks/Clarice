export PATH=/mnt/Audit/tools:/bin
cd /mnt/Audit/help
status runtime > before
cmp binary binary > t0.out 2> t0.err
echo $? > t0.status
cmp -s binary changed > t1.out 2> t1.err
echo $? > t1.status
cmp -l binary changed > t2.out 2> t2.err
echo $? > t2.status
cmp empty empty > t3.out 2> t3.err
echo $? > t3.status
cmp binary absent > t4.out 2> t4.err
echo $? > t4.status
cmp -l -s binary binary > t5.out 2> t5.err
echo $? > t5.status
cksum binary changed empty > t6.out 2> t6.err
echo $? > t6.status
cksum < binary > t7.out 2> t7.err
echo $? > t7.status
cat binary | cksum > t8.out 2> t8.err
echo $? > t8.status
cmp - binary < binary > t9.out 2> t9.err
echo $? > t9.status
cmp --bad-option > t10.out 2> t10.err
echo $? > t10.status
cksum --bad-option > t11.out 2> t11.err
echo $? > t11.status
awk --bad-option > t12.out 2> t12.err
echo $? > t12.status
ls --bad-option > t13.out 2> t13.err
echo $? > t13.status
find --bad-option > t14.out 2> t14.err
echo $? > t14.status
serial --bad-option > t15.out 2> t15.err
echo $? > t15.status
i=0; while [ "$i" -lt 12 ]; do cmp -s binary binary; cksum binary > repeat; i=$((i+1)); done
status runtime > after
echo TOOLS_DONE
