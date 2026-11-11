mkdir /tmp/qol
cd '/mnt/Test 2'
pwd > /tmp/qol/pwd.txt
ls -la > /tmp/qol/relative.txt
ls -la '/mnt/Test 2' > /tmp/qol/absolute.txt
cd /
cd /mnt/Test\ 2
pwd > /tmp/qol/escaped.txt
cat sentinel.txt > /tmp/qol/read.txt
ls -la '/mnt/Test 2/sub' > /tmp/qol/sub.txt
echo QOL-BASELINE-DONE
