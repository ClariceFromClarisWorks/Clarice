: > /tmp/res-results
/tmp/rescheck /tmp/res-source >> /tmp/res-results
echo source:$? >> /tmp/res-results
for v in before after; do
 /tmp/fp-$v/cp -p /tmp/res-source /tmp/res-$v
 echo $v-create:$? >> /tmp/res-results
 /tmp/rescheck /tmp/res-$v >> /tmp/res-results
 echo $v-read-create:$? >> /tmp/res-results
 /tmp/fp-$v/cp -p /tmp/res-source /tmp/res-$v
 echo $v-replace:$? >> /tmp/res-results
 /tmp/rescheck /tmp/res-$v >> /tmp/res-results
 echo $v-read-replace:$? >> /tmp/res-results
 /tmp/fp-$v/mv /tmp/res-$v /tmp/res-renamed-$v
 echo $v-rename:$? >> /tmp/res-results
 /tmp/rescheck /tmp/res-renamed-$v >> /tmp/res-results
 echo $v-read-rename:$? >> /tmp/res-results
 /tmp/fp-$v/mv /tmp/res-renamed-$v /mnt/GlyphFull/res-$v
 echo $v-move:$? >> /tmp/res-results
 /tmp/rescheck /mnt/GlyphFull/res-$v >> /tmp/res-results
 echo $v-read-move:$? >> /tmp/res-results
 /tmp/fp-$v/cp -p /mnt/GlyphFull/res-$v /tmp/res-back-$v
 echo $v-return:$? >> /tmp/res-results
 /tmp/rescheck /tmp/res-back-$v >> /tmp/res-results
 echo $v-read-return:$? >> /tmp/res-results
 /tmp/ls-flush /mnt/GlyphFull/res-$v
 echo $v-flush:$? >> /tmp/res-results
done
/tmp/rescheck /tmp/res-source >> /tmp/res-results
echo source-after:$? >> /tmp/res-results
/tmp/ls-flush /tmp/res-results
echo RESOURCE-TESTS-DONE
