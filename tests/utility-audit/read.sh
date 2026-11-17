cd /mnt/Audit
f() { read x; echo first:$?:$x; read x; echo second:$?:$x; }
x=old
echo read127
f < read127
x=old
echo read128
f < read128
x=old
echo readlong
f < readlong
echo READ_DONE
