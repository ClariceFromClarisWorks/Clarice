printf 'PB_BEGIN\n'
command -v clarice
if test -f /bin/clarice; then
 if test -e /etc/clarice/legacy-updater-1882.clx; then
  echo ARCHIVE_EXISTS
  exit 1
 fi
 mv /bin/clarice /etc/clarice/legacy-updater-1882.clx || exit 1
fi
test ! -f /bin/clarice || exit 1
time clarice --help
printf 'HELP1=%d\n' $?
status runtime
time clarice --help
printf 'HELP2=%d\n' $?
status runtime
time clarice update -y --source http://192.168.1.112:8098/self-update-1902/repository/index.txt
printf 'UPDATE=%d\n' $?
status runtime
test ! -f /bin/clarice
printf 'NO_EXTERNAL=%d\n' $?
test ! -f /etc/clarice/update.incomplete
printf 'NO_PENDING=%d\n' $?
printf 'b\na\n' > /tmp/pb180-su-input
sort /tmp/pb180-su-input
wc -l /tmp/pb180-su-input
printf 'PB_END\n'
