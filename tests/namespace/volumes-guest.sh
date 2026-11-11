# Disposable Sys6Unix boot copy and Test 2 fixture only.
cd '/mnt/Test 2'
pwd
ls -1 .
ls -la '/mnt/Test 2'
cd /
cd /mnt/Test\ 2
pwd
cat sentinel.txt
ls -1 sub
ls -1 '/mnt/Test 2/sub'
cd sub
pwd
cat second.txt
cd ..
pwd
cd /mnt/Sys6Unix
pwd
ls -1 /mnt/Sys6Unix/qol-safe
cat /mnt/Sys6Unix/qol-safe/guard
cd '/mnt/Test 2'
mkdir qol-safe
echo SECOND > qol-safe/guard
cat qol-safe/guard
cp qol-safe/guard qol-safe/copied
mv qol-safe/copied qol-safe/renamed
cat qol-safe/renamed
touch qol-safe/touched
ls -1 qol-safe
rm qol-safe/renamed
rm qol-safe/touched
mkdir qol-safe/empty
rmdir qol-safe/empty
ln -s '/mnt/Test 2/qol-safe/guard' /tmp/qol/cross-link
cat /tmp/qol/cross-link
cp qol-safe/guard /mnt/Sys6Unix/qol-safe/from-second
cat /mnt/Sys6Unix/qol-safe/from-second
cat /mnt/Sys6Unix/qol-safe/guard
ls -1 /mnt/Sys6Unix/qol-safe
cd /mnt/MissingVolume
 echo invalid-cd=$?
pwd
ls /mnt/MissingVolume
 echo invalid-ls=$?
echo BAD > /mnt/MissingVolume/qol-safe/guard
 echo invalid-write=$?
rm /mnt/MissingVolume/qol-safe/guard
 echo invalid-rm=$?
cd '/mnt/Test 2/absent'
 echo absent-cd=$?
pwd
ls '/mnt/Test 2/absent'
 echo absent-ls=$?
echo BAD > '/mnt/Test 2/absent/guard'
 echo absent-write=$?
cat /mnt/Sys6Unix/qol-safe/guard
cat '/mnt/Test 2/qol-safe/guard'
ls -1 '/mnt/Test 2/qol-safe'
cd /
benchscan '/mnt/Test 2'
benchscan /mnt/Sys6Unix/Bench/n1000
echo QOL-REGRESSION-DONE
