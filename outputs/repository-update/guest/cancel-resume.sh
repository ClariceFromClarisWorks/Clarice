test -f /etc/clarice/update.incomplete || exit 1
test ! -f /bin/reposlow || exit 2
clarice update -y --source http://192.168.1.112:8083/slow.txt || exit 3
test ! -f /etc/clarice/update.incomplete || exit 4
appinstall check /bin/reposlow || exit 5
echo CANCEL-RESUME-PASS
