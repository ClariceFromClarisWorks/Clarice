echo CORRUPT-BEGIN
clarice update -y --source http://192.168.1.112:8083/corrupt.txt
rc=$?
test "$rc" = 1 || exit 1
test -f /etc/clarice/update.incomplete || exit 2
cmp /bin/repoone /bin/free || exit 3
appinstall check /bin/repoone || exit 4
echo CORRUPT-PRESERVED-PASS
