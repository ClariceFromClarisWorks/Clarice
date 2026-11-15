mkdir native-check
echo MKDIR:$?
printf 'payload\n' > native-check/file
hfsattr --type TEXT --creator TEST native-check/file
echo ATTR:$?
file native-check/file
ls -a native-check
ls -lh native-check/file
ls -Z native-check > /mnt/Audit/ls-invalid.out 2> /mnt/Audit/ls-invalid.err
echo LS_INVALID:$?
df -h /mnt/Audit > /mnt/Audit/df-current.out
echo DF:$?
du -sk native-check
hfsattr --locked on native-check/file
rm native-check/file
echo LOCKED:$?
hfsattr --locked off native-check/file
rm -r native-check
echo REMOVE:$?
