mkdir /tmp/full1713 || exit 1
wget -O /tmp/full1713/stage ${DEPLOY_SOURCE:?set trusted local deployment URL}/full1713/stage || exit 1
wget -O /tmp/full1713/release.tar ${DEPLOY_SOURCE:?set trusted local deployment URL}/full1713/release.tar || exit 1
mkdir /tmp/full1713/files || exit 1
/tmp/full1713/stage extract /tmp/full1713/release.tar /tmp/full1713/files > /tmp/full1713/extract.log || exit 1
for n in ls df cat benchls benchscan grep sed awk serial wc head cut tr uniq tee tail sort cp mv touch rmdir mkdir rm ln readlink hfsattr find du file od more basename dirname kermit wget clarice cmp cksum free sleep printenv mktemp dd ipconfig ping ftp telnet; do
 appinstall check /tmp/full1713/files/$n || exit 1
done
echo PREPARE-47-PASS
