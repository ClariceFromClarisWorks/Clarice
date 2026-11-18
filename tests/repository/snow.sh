# Execute on disposable Snow media only. Explicit guards: no dependence on set -e.
echo REPOSITORY-BEGIN
clarice update --check || exit 1
clarice update -y || exit 2
test -f /bin/repoone || exit 3
test -f /bin/repotwo || exit 4
test ! -f /etc/clarice/update.incomplete || exit 5
appinstall check /bin/repoone || exit 6
appinstall check /bin/repotwo || exit 7
cat /etc/clarice/installed.catalog || exit 8
clarice update -y || exit 9
echo REPOSITORY-REPEAT-PASS
status runtime
echo REPOSITORY-END
