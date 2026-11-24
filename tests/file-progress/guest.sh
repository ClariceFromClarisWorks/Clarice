export PATH=/tmp/fp-after:/bin:/usr/bin
: > /tmp/fp-results
for r in 0 1 2 3; do
 for v in before after; do
  echo copy:$r:$v >> /tmp/fp-results
  time /tmp/fp-$v/cp -p /tmp/fp-source /tmp/fp-copy-$v 2>> /tmp/fp-results
  echo status:$? >> /tmp/fp-results
  status runtime >> /tmp/fp-results
 done
 cmp /tmp/fp-copy-before /tmp/fp-copy-after
 echo compare:$? >> /tmp/fp-results
done
mv /tmp/fp-copy-after /tmp/fp-renamed
echo rename:$? >> /tmp/fp-results
mv /tmp/fp-renamed /mnt/GlyphFull/fp-moved
echo cross-move:$? >> /tmp/fp-results
cmp /tmp/fp-source /mnt/GlyphFull/fp-moved
echo moved-data:$? >> /tmp/fp-results
cp -R /tmp/fp-small /tmp/fp-tree
echo tree-copy:$? >> /tmp/fp-results
rm -R /tmp/fp-tree
echo tree-remove:$? >> /tmp/fp-results
test ! -e /tmp/fp-tree
echo tree-absent:$? >> /tmp/fp-results
printf KEEP > /tmp/fp-locked
hfsattrib --locked on /tmp/fp-locked
cp /tmp/fp-source /tmp/fp-locked 2>> /tmp/fp-results
echo locked-copy:$? >> /tmp/fp-results
rm -f /tmp/fp-locked 2>> /tmp/fp-results
echo locked-remove:$? >> /tmp/fp-results
hfsattrib --locked off /tmp/fp-locked
rm /tmp/fp-locked
echo unlock-remove:$? >> /tmp/fp-results
cp --no-progress -p /tmp/fp-source /tmp/fp-quiet
echo quiet-copy:$? >> /tmp/fp-results
cp --bogus /tmp/fp-source /tmp/fp-bad 2>> /tmp/fp-results
echo invalid:$? >> /tmp/fp-results
rm --no-progress /tmp/fp-quiet
echo quiet-remove:$? >> /tmp/fp-results
/tmp/ls-flush /tmp/fp-results
/tmp/ls-flush /mnt/GlyphFull/fp-moved
echo FILE-PROGRESS-TESTS-DONE
