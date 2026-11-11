appinstall replace /mnt/Task2/probe1.clx acceptprobe > /tmp/tasktwo/accept-install.txt 2>&1
echo $? > /tmp/tasktwo/accept-install-status.txt
acceptprobe > /tmp/tasktwo/accept-one.txt
hfsattr --type TEST --creator TST2 --invisible on /bin/acceptprobe
hfsattr /bin/acceptprobe > /tmp/tasktwo/accept-metadata-before.txt
appinstall replace /mnt/Task2/probe2.clx acceptprobe > /tmp/tasktwo/accept-replace.txt 2>&1
echo $? > /tmp/tasktwo/accept-replace-status.txt
acceptprobe > /tmp/tasktwo/accept-two.txt
hfsattr /bin/acceptprobe /bin/.clx-acceptprobe.old > /tmp/tasktwo/accept-metadata-after.txt
appinstall recover acceptprobe > /tmp/tasktwo/accept-recover.txt 2>&1
echo $? > /tmp/tasktwo/accept-recover-status.txt
acceptprobe > /tmp/tasktwo/accept-recovered.txt
echo FOURDONE
