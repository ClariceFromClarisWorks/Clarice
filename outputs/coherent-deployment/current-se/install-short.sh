for n in ls df cat benchls benchscan grep sed awk serial wc head cut tr uniq tee tail sort cp mv touch rmdir mkdir rm ln readlink hfsattr find du file od more basename dirname kermit wget clarice cmp cksum free sleep printenv mktemp dd ipconfig ping ftp telnet; do
 appinstall replace /tmp/full1713/files/$n $n || exit 1
done
/tmp/full1713/stage verify /tmp/full1713/release.tar /bin || exit 1
echo INSTALL-47-PASS
