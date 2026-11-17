echo RESUME-BEGIN
clarice update -y --source http://192.168.1.112:8083/partial.txt || exit 1
status runtime
test ! -f /etc/clarice/update.incomplete || exit 2
cmp /bin/repobad /bin/free || exit 3
cmp /bin/repoone /bin/free || exit 4
grep 'keep me' /etc/clarice/repo-user.txt || exit 5
clarice update -y --source http://192.168.1.112:8083/partial.txt || exit 6
status runtime
clarice update -y --source http://192.168.1.112:8083/partial.txt || exit 7
status runtime
echo RESUME-PASS
