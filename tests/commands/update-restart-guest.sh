# Run after restarting Clarice, on the same disposable Task2 test disk.
task2probe > /tmp/task2-results/restarted.txt
cp /bin/grep /tmp/task2-results/grep-after-restart.clx
appinstall recover task2probe > /tmp/task2-results/recover.txt 2>&1
echo $? > /tmp/task2-results/recover-status.txt
task2probe > /tmp/task2-results/recovered.txt
appinstall recover grep > /tmp/task2-results/grep-recover.txt 2>&1
mv /bin/.clx-grep.new /tmp/task2-results/grep-retained.clx
appinstall repair grep > /tmp/task2-results/repair.txt 2>&1
echo $? > /tmp/task2-results/repair-status.txt
echo alpha | grep alpha > /tmp/task2-results/grep-repaired.txt
echo alpha | sed s/alpha/beta/ > /tmp/task2-results/sed.txt
echo 'alpha beta' | awk '{print $2}' > /tmp/task2-results/awk.txt
df -h > /tmp/task2-results/df.txt
cat /tmp/task2-results/restarted.txt /tmp/task2-results/recovered.txt
echo 'Expected: independent command v2, then independent command v1'
echo 'Timing: run cursesdemo, quit with Q, then status runtime; repeat twice.'
