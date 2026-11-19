# Run locally in the disposable Snow guest after installing the candidate Core.
# /mnt/GlyphFull/Share Repo contains probe.clx and the fixture indexes.
clarice --help
clarice update -y --source '/mnt/GlyphFull/Share Repo/partial.txt' > /tmp/share-partial.log
printf 'PARTIAL=%s\n' $?
test -f /etc/clarice/update.incomplete
printf 'MARKER=%s\n' $?
shareone --help
clarice update -y --source '/mnt/GlyphFull/Share Repo/resume.txt' > /tmp/share-resume.log
printf 'RESUME=%s\n' $?
test ! -f /etc/clarice/update.incomplete
printf 'COMPLETE=%s\n' $?
sharetwo --help
clarice update -y --source '/mnt/GlyphFull/Share Repo/corrupt.txt' > /tmp/share-corrupt.log
printf 'CORRUPT=%s\n' $?
shareone --help
clarice update -y --source '/mnt/GlyphFull/Share Repo/resume.txt' > /tmp/share-repeat.log
printf 'REPEAT=%s\n' $?
cat /tmp/share-user.txt
status runtime
printf 'SHARE_TEST_DONE\n'
