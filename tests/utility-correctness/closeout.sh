echo PROFILE_STATUS:$?
mkdir /tmp/closeout
mkdir /tmp/closeout/a
mkdir /tmp/closeout/b
printf 'pear\napple\npear\n' > /tmp/closeout/a/fruit.txt
printf 'note\n' > '/tmp/closeout/b/file space.txt'
find /tmp/closeout -name '*.txt' -name 'fruit*' | sort
find /tmp/closeout -type f -type d
echo TYPES:$?
find /tmp/closeout/a /tmp/closeout/b -path '*b/*' -type f | sort
find /tmp/closeout/absent /tmp/closeout/a -maxdepth 0
echo MISSING:$?
find /tmp/closeout -type invalid
echo INVALID:$?
for p in /tmp/closeout/a/*.txt; do cat "$p"; done | sort | uniq -c
find /tmp/closeout -type f | sort | while read -r p; do printf '<%s>\n' "$p"; done
rm -r /tmp/closeout
echo CLEANUP:$?
status runtime
echo CLOSEOUT_DONE
