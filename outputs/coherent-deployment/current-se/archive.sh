mkdir /etc/clarice/recovery-pre1713 || exit 1
for n in benchls clarice kermit ls serial wget; do
 mv /bin/.clx-$n.old /etc/clarice/recovery-pre1713/.clx-$n.old || exit 1
done
echo ARCHIVE-6-PASS
