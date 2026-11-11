appinstall check /bin/grep > /tmp/tasktwo/damaged-restarted.txt 2>&1
echo $? > /tmp/tasktwo/damaged-restarted-status.txt
cp /bin/grep /tmp/tasktwo/damaged-restarted.clx
appinstall repair grep > /tmp/tasktwo/repair.txt 2>&1
echo $? > /tmp/tasktwo/repair-status.txt
echo alpha | grep alpha > /tmp/tasktwo/grep-repaired.txt
appinstall check /bin/.clx-grep.old > /tmp/tasktwo/old-damage.txt 2>&1
echo $? > /tmp/tasktwo/old-damage-status.txt
cp /bin/.clx-grep.old /tmp/tasktwo/old-damage.clx
ls /bin > /tmp/tasktwo/ls.txt
echo $? > /tmp/tasktwo/ls-status.txt
df -h > /tmp/tasktwo/df.txt
echo $? > /tmp/tasktwo/df-status.txt
echo alpha | sed s/alpha/beta/ > /tmp/tasktwo/sed.txt
echo 'alpha beta' | awk '{print $2}' > /tmp/tasktwo/awk.txt
echo THIRDDONE
