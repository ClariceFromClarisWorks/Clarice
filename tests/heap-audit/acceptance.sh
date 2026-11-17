export PATH=/mnt/Audit/tools:/bin
cd /mnt/Audit/practical
val=$(printf '%02048d' 0)
PRIVATE=hidden
export TERM=wrong
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39; do export "E$i=$val"; done
for i in 1 2 3; do
 /mnt/Audit/env-fault 10000
 echo FAILURE:$?
 status runtime
 /mnt/Audit/environment
 echo RECOVERY:$?
 status runtime
done
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39; do unset "E$i"; done
unset val
for i in 1 2 3; do
 awk 'BEGIN {'
 echo AWK_FAILURE:$?
 awk 'BEGIN { for(i=0;i<1000;i++)a[i]=i; print a[999] }'
 echo LARGE:$?
 status runtime
done
awk 'BEGIN { for(i=0;i<100000;i++)a[i]=i }'
echo EXHAUSTION:$?
status runtime
awk 'BEGIN { print "AWK_RECOVERED" }'
echo AFTER_EXHAUSTION:$?
status runtime
for i in 1 2 3 4; do awk 'BEGIN {}' > /dev/null; /mnt/Audit/snapshot > "/mnt/Audit/practical/fixed-walk-$i"; done
