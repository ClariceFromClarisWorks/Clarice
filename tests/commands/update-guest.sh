# Disposable Plus 4 MB / System 6.0.8 fixture only. Run before-restart first.
# Fixture files are on /mnt/Task2; results remain in /tmp/task2-results.
# Do not use personal media: the script intentionally replaces /bin/grep.
mkdir /tmp/task2-results
appinstall check /mnt/Task2/probe2.clx > /tmp/task2-results/check.txt 2>&1
echo $? > /tmp/task2-results/check-status.txt
appinstall replace /mnt/Task2/probe1.clx task2probe > /tmp/task2-results/install.txt 2>&1
echo $? > /tmp/task2-results/install-status.txt
task2probe > /tmp/task2-results/probe1.txt
appinstall replace /mnt/Task2/probe2.clx task2probe > /tmp/task2-results/replace.txt 2>&1
echo $? > /tmp/task2-results/replace-status.txt
task2probe > /tmp/task2-results/probe2.txt
appinstall replace /mnt/Task2/incompatible.clx task2bad > /tmp/task2-results/incompatible.txt 2>&1
echo $? > /tmp/task2-results/incompatible-status.txt
appinstall replace /mnt/Task2/corrupt.clx task2bad > /tmp/task2-results/corrupt.txt 2>&1
echo $? > /tmp/task2-results/corrupt-status.txt
appinstall replace /mnt/Task2/grep2.clx grep > /tmp/task2-results/grep-update.txt 2>&1
echo $? > /tmp/task2-results/grep-update-status.txt
echo alpha | grep alpha > /tmp/task2-results/grep.txt
ls /bin > /tmp/task2-results/ls.txt
status runtime > /tmp/task2-results/ls-timing.txt
echo 'Quit and restart Clarice, then run /mnt/Task2/after.sh'
