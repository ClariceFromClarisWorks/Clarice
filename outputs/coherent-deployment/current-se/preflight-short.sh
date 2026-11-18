for n in ls df cat benchls benchscan grep sed awk serial wc head cut tr uniq tee tail sort cp mv touch rmdir mkdir rm ln readlink hfsattr find du file od more basename dirname kermit wget clarice cmp cksum free sleep printenv mktemp dd ipconfig ping ftp telnet; do
 if test -e /bin/.clx-$n.old; then echo BLOCKED-$n-old; exit 1; fi
 if test -e /bin/.clx-$n.new; then echo BLOCKED-$n-new; exit 1; fi
done
echo PREFLIGHT-PASS
