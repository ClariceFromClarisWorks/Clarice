clarice update -y --source http://192.168.1.112:8100/tools2.txt > /tmp/su2.log
printf 'REPLACE=%s\n' $?
status runtime
selfprobe
clarice update -y --source http://192.168.1.112:8100/bad.txt > /tmp/su-bad.log
printf 'CORRUPT=%s\n' $?
status runtime
selfprobe
clarice update -y --source http://192.168.1.112:8100/tools1.txt > /tmp/su-old.log
printf 'NO_DOWNGRADE=%s\n' $?
selfprobe
test ! -f /bin/clarice
printf 'NO_UPDATER_FILE=%s\n' $?
test ! -f /etc/clarice/update.incomplete
printf 'NO_PENDING=%s\n' $?
