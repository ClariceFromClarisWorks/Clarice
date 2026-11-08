PATH=/usr/bin ls
echo $? > /tmp/no-path-status.txt
/bin/ls -1 /bin > /tmp/bin-list.txt
ls -Z 2> /tmp/usage.txt
echo $? > /tmp/usage-status.txt
df -h > /tmp/df-human.txt
printf '#!/bin/sh\necho shadow\n' > /tmp/ls
PATH=/tmp:/bin ls > /tmp/shadow.txt
/bin/ls -1 /tmp > /tmp/explicit.txt
cat /etc/profile > /tmp/profile-copy.txt
probe one
echo $? > /tmp/probe1.txt
probe one two
echo $? > /tmp/probe2.txt
badcommand
echo $? > /tmp/bad-status.txt
missingcommand 2> /tmp/missing-command.txt
echo $? > /tmp/missing-command-status.txt
echo PATH_COMPLETE
