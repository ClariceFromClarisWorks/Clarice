cd /
pwd
cd /mnt
pwd
ls /mnt
cd /mnt/Sys6Unix/Documents
pwd
cat '/mnt/Sys6Unix/Documents/with space.txt'
cd /
ln -s /mnt/Sys6Unix/Documents /tmp/media
cd /tmp/media
pwd
cd /
mkdir '/tmp/space dir'
echo spaces > '/tmp/space dir/file'
cat '/tmp/space dir/file'
echo encoded > /tmp/space%20file
cat '/tmp/space file'
cd /mnt/Sys6Unix/..
pwd
cd /../../..
pwd
