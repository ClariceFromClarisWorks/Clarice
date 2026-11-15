# Disposable listing fixtures on the two test volumes only.
mkdir /tmp/catalog-test
mkdir '/mnt/Test 2/catalog-test'
echo BOOT > /tmp/catalog-test/guard
echo SECOND > '/mnt/Test 2/catalog-test/guard'
mkdir '/mnt/Test 2/catalog-test/sub'
touch '/mnt/Test 2/catalog-test/space name'
touch '/mnt/Test 2/catalog-test/.hidden'
touch '/mnt/Test 2/catalog-test/percent%25name'
cd '/mnt/Test 2/catalog-test'
pwd
ls -1
ls -la
ls -1 /tmp/catalog-test
ls -1 '/mnt/Test 2/catalog-test'
cat guard
cat /tmp/catalog-test/guard
ls /mnt/MissingVolume
echo absent=$?
ls '/mnt/Test 2/catalog-test/missing'
echo missing=$?
cd /tmp/catalog-test
pwd
cat guard
cd /
rm '/mnt/Test 2/catalog-test/guard' '/mnt/Test 2/catalog-test/space name' '/mnt/Test 2/catalog-test/.hidden' '/mnt/Test 2/catalog-test/percent%25name'
rmdir '/mnt/Test 2/catalog-test/sub' '/mnt/Test 2/catalog-test'
rm /tmp/catalog-test/guard
rmdir /tmp/catalog-test
echo CATALOG-GUEST-DONE
