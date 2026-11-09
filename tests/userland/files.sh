echo FILES-START
mkdir -p /tmp/ul-test/sub
echo mkdir:$?
touch /tmp/ul-test/new
hfsattr /tmp/ul-test/new
hfsattr --type TEXT --creator TEST /tmp/ul-test/new
touch /tmp/ul-test/new
hfsattr /tmp/ul-test/new
cp -p /tmp/fork-source /tmp/ul-test/copy
echo copy:$?
file /tmp/ul-test/copy
cp /tmp/pipeline-data /tmp/ul-test/copy
echo overwrite:$?
file /tmp/ul-test/copy
cp -p /tmp/fork-source /tmp/ul-test/copy
mv /tmp/ul-test/copy /tmp/ul-test/renamed
echo rename:$?
mv /tmp/ul-test/renamed /tmp/ul-test/sub
echo move:$?
file /tmp/ul-test/sub/renamed
ln -s sub/renamed /tmp/ul-test/link
readlink /tmp/ul-test/link
cat /tmp/ul-test/link
hfsattr --locked on /tmp/ul-test/new
rm /tmp/ul-test/new
echo locked:$?
hfsattr --locked off --invisible on /tmp/ul-test/new
hfsattr /tmp/ul-test/new
rmdir /tmp/ul-test
echo nonempty:$?
chmod 755 /tmp/ul-test/new
echo chmod:$?
echo FILES-END
touch /tmp/ul-missing/child
echo missing-parent:$?
file /tmp/ul-missing
echo parent-absent:$?
