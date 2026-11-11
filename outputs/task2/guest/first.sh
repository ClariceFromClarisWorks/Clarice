mkdir /tmp/tasktwo
appinstall --help > /tmp/tasktwo/help.txt
wc --help > /tmp/tasktwo/fresh-wc.txt
echo $? > /tmp/tasktwo/fresh-wc-status.txt
hfsattr --type TEST --creator TST2 --invisible on /bin/grep
echo $? > /tmp/tasktwo/metadata-set-status.txt
hfsattr /bin/grep > /tmp/tasktwo/metadata-before.txt
appinstall replace /mnt/Task2/grep2.clx grep > /tmp/tasktwo/replace.txt 2>&1
echo $? > /tmp/tasktwo/replace-status.txt
hfsattr /bin/grep /bin/.clx-grep.old > /tmp/tasktwo/metadata-replaced.txt
cp /bin/grep /tmp/tasktwo/updated-grep.clx
echo alpha | grep alpha > /tmp/tasktwo/grep-before.txt
appinstall replace /mnt/Task2/probe1.clx task2probe > /tmp/tasktwo/probe-install.txt 2>&1
echo $? > /tmp/tasktwo/probe-install-status.txt
appinstall replace /mnt/Task2/probe2.clx task2probe > /tmp/tasktwo/probe-replace.txt 2>&1
echo $? > /tmp/tasktwo/probe-replace-status.txt
task2probe > /tmp/tasktwo/probe-before.txt
appinstall replace /mnt/Task2/incompatible.clx task2bad > /tmp/tasktwo/incompatible.txt 2>&1
echo $? > /tmp/tasktwo/incompatible-status.txt
echo FIRSTDONE
