mkdir /tmp/operands
printf 'kept\n' > /tmp/operands/file
set --
i=0
while test "$i" -lt 100; do set -- "$@" /tmp/operands/file; i=$((i+1)); done
ls -1P "$@" > /mnt/Audit/many-ls
echo LS:$?
df "$@" > /mnt/Audit/many-df
echo DF:$?
ls -Z "$@" > /mnt/Audit/bad-ls
echo BAD_LS:$?
for n in 1 2 3; do
 find /tmp/operands -name 'f*' -name '*e' -type f > /mnt/Audit/found
 echo FIND:$?
 status runtime
done
rm -r /tmp/operands
echo CLEANUP:$?
echo OPERANDS_DONE
