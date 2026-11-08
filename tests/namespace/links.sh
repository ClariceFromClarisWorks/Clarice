mkdir /tmp/ns
mkdir /tmp/ns/dir
echo original > /tmp/ns/dir/data
ln -s dir/data /tmp/ns/relative
cat /tmp/ns/relative
ln -s /tmp/ns/relative /tmp/ns/absolute
cat /tmp/ns/absolute
ln -s dir /tmp/ns/folder
cat /tmp/ns/folder/data
cd /tmp/ns/folder
pwd
cd ..
pwd
readlink /tmp/ns/relative
echo changed > /tmp/ns/absolute
cat /tmp/ns/dir/data
rm /tmp/ns/absolute
test -f /tmp/ns/relative && echo target-preserved
test -L /tmp/ns/relative && echo link-type
ln -s missing /tmp/ns/dangling
test -L /tmp/ns/dangling && echo dangling-link
if test -e /tmp/ns/dangling; then echo FAIL; else echo dangling-not-file; fi
echo created > /tmp/ns/dangling
cat /tmp/ns/missing
ln -s b /tmp/ns/a
ln -s a /tmp/ns/b
if test -e /tmp/ns/a; then echo FAIL; else echo loop-bounded; fi
rm /tmp/ns/a
rm /tmp/ns/b
