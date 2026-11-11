cp /bin/grep /tmp/tasktwo/restarted-grep.clx
hfsattr /bin/grep /bin/.clx-grep.old > /tmp/tasktwo/metadata-restarted.txt
echo alpha | grep alpha > /tmp/tasktwo/grep-restarted.txt
task2probe > /tmp/tasktwo/probe-restarted.txt
appinstall recover task2probe > /tmp/tasktwo/probe-recover.txt 2>&1
echo $? > /tmp/tasktwo/probe-recover-status.txt
task2probe > /tmp/tasktwo/probe-recovered.txt
appinstall recover grep > /tmp/tasktwo/recover.txt 2>&1
echo $? > /tmp/tasktwo/recover-status.txt
hfsattr /bin/grep /bin/.clx-grep.new > /tmp/tasktwo/metadata-recovered.txt
cp /bin/grep /tmp/tasktwo/recovered-grep.clx
mv /bin/.clx-grep.new /tmp/tasktwo/retained-grep.clx
echo $? > /tmp/tasktwo/archive-status.txt
cp /mnt/Task2/corrupt.clx /bin/grep
echo $? > /tmp/tasktwo/damage-copy-status.txt
appinstall check /bin/grep > /tmp/tasktwo/damaged-before.txt 2>&1
echo $? > /tmp/tasktwo/damaged-before-status.txt
cp /bin/grep /tmp/tasktwo/damaged-before.clx
echo SECONDDONE
