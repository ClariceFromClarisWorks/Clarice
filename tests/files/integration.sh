#!/bin/sh
ls -a /bin > /tmp/dots.txt
ls -1 /mnt > /tmp/mounts.txt
ls -lh /mnt/sys6unix/clarice > /tmp/forks.txt
df > /tmp/df.txt
df -h / > /tmp/dfh.txt
ls -a /bin /usr/bin > /tmp/multiple.txt
ls -- /bin > /tmp/empty.txt
ls -a /mnt/sys6unix/bench/n1000 > /tmp/thousand.txt
ls -Z > /tmp/invalid.txt
echo $? > /tmp/invalid-status.txt
df /does-not-exist
echo $? > /tmp/missing-status.txt
echo FILE_TEST_COMPLETE
