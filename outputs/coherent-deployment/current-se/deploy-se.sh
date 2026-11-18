/tmp/full1713/files/cksum /etc/clarice/clarice.conf /etc/profile > /tmp/full1713/config-before.txt || exit 1
mkdir /etc/clarice/recovery-pre1713 || exit 1
for n in benchls clarice kermit ls serial wget; do
 /tmp/full1713/files/mv /bin/.clx-$n.old /etc/clarice/recovery-pre1713/.clx-$n.old || exit 1
done
echo ARCHIVE-6-PASS
for n in ls df cat benchls benchscan grep sed awk serial wc head cut tr uniq tee tail sort cp mv touch rmdir mkdir rm ln readlink hfsattr find du file od more basename dirname kermit wget clarice cmp cksum free sleep printenv mktemp dd ipconfig ping ftp telnet; do
 if test -e /bin/.clx-$n.old; then echo BLOCKED-$n-old; exit 1; fi
 if test -e /bin/.clx-$n.new; then echo BLOCKED-$n-new; exit 1; fi
done
echo PREFLIGHT-PASS
for n in ls df cat benchls benchscan grep sed awk serial wc head cut tr uniq tee tail sort cp mv touch rmdir mkdir rm ln readlink hfsattr find du file od more basename dirname kermit wget clarice cmp cksum free sleep printenv mktemp dd ipconfig ping ftp telnet; do
 appinstall replace /tmp/full1713/files/$n $n || exit 1
done
/tmp/full1713/stage verify /tmp/full1713/release.tar /bin || exit 1
echo INSTALL-47-PASS
cksum /etc/clarice/clarice.conf /etc/profile > /tmp/full1713/config-after.txt || exit 1
cmp /tmp/full1713/config-before.txt /tmp/full1713/config-after.txt || exit 1
echo SETTINGS-PASS
printf test | cksum
printf 'hello\n' | grep hello || exit 1
printf 'alpha\n' | sed s/alpha/beta/ || exit 1
printf '2\n3\n' | awk '{s += $1} END {print s}' || exit 1
cmp /bin/cmp /tmp/full1713/files/cmp || exit 1
ftp --help > /tmp/full1713/ftp-help.txt || exit 1
telnet --help > /tmp/full1713/telnet-help.txt || exit 1
ipconfig || exit 1
status runtime
echo SMOKE-PASS
echo DEPLOYMENT-1713-COMPLETE
