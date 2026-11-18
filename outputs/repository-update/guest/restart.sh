echo RESTART-BEGIN
clarice update -y --source http://192.168.1.112:8083/restart.txt
rc=$?
test "$rc" = 1 || exit 1
test -f /etc/clarice/update.incomplete || exit 2
appinstall check /bin/repoone || exit 3
echo RESTART-READY
