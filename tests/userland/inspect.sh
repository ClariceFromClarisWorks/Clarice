echo INSPECT-START
cp -R /tmp/ul-test /tmp/ul-copy
echo recursive:$?
file /tmp/ul-copy/sub/renamed
readlink /tmp/ul-copy/link
find /tmp/ul-copy -type f | sort
find /tmp/ul-copy -name '*name*'
du -s /tmp/ul-copy
du -sh /tmp/ul-copy
file /tmp/ul-copy/link
od -A x -t x1 -N 12 /tmp/fork-source
od -A d -t c -j 9 -N 5 /tmp/fork-source
basename /mnt/volume/folder/file.txt .txt
dirname /mnt/volume/folder/file.txt
more /tmp/pipeline-data > /tmp/ul-more
wc -c /tmp/ul-more
cp /tmp/ul-copy/sub/renamed /tmp/ul-copy/sub/renamed
echo selfcopy:$?
cp -R /tmp/ul-copy /tmp/ul-copy/nested
echo descendant:$?
rm -r /tmp/ul-copy
echo remove:$?
touch -c /tmp/ul-absent
file /tmp/ul-absent
echo absent:$?
mkdir /tmp/ul-empty
rmdir /tmp/ul-empty
echo rmdir:$?
echo INSPECT-END
