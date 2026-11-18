echo PARTIAL-BEGIN
printf 'keep me\n' > /etc/clarice/repo-user.txt || exit 1
clarice update -y --source http://192.168.1.112:8083/partial.txt
rc=$?
test "$rc" = 1 || exit 2
test -f /etc/clarice/update.incomplete || exit 3
grep 'repoone' /etc/clarice/installed.catalog || exit 4
appinstall check /bin/repoone || exit 5
test -f /bin/.clx-repoone.old || exit 6
cmp /bin/repoone /bin/free || exit 7
echo PARTIAL-FAILURE-PASS
status runtime
