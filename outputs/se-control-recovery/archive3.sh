p='/mnt/Macintosh HD'
a="$p/Clarice Archive 20261117"
active=$(/tmp/id50) || exit 1
test "$active" = 'Macintosh HD:.cu-0000039b-78c4021b.old' || exit 1
test -d "$a" || exit 1
set --
for s in "$p"/.cu-*.old "$p/Clarice" "$p/Clarice Update" "$p/Clarice UpdatstClariceuste" "$p/Clarice Update.bin" "$p/Update2.bin"; do
 case "$s" in
  "$p/.cu-0000039b-78c4021b.old"|"$p/.cu-0000065a-78c402e0.old"|"$p/.cu-0000065d-78c402e1.old") continue ;;
 esac
 test -f "$s" || continue
 n=${s##*/}
 test ! -e "$a/$n" || exit 1
 set -- "$@" "$s"
done
echo ARCHIVE_REMAINING "$#"
test "$#" -gt 0 || exit 1
file "$@" "$a"/.cu-*.old "$a"/* > /tmp/archive-before.txt || exit 1
hfsattr "$@" "$a"/.cu-*.old "$a"/* >> /tmp/archive-before.txt || exit 1
mv "$@" "$a" || exit 1
file "$a"/.cu-*.old "$a"/* > /tmp/archive-after.txt || exit 1
hfsattr "$a"/.cu-*.old "$a"/* >> /tmp/archive-after.txt || exit 1
sed 's|/mnt/Macintosh HD/Clarice Archive 20261117/||; s|/mnt/Macintosh HD/||' /tmp/archive-before.txt > /tmp/archive-before.paths || exit 1
sort /tmp/archive-before.paths > /tmp/archive-before.norm || exit 1
sed 's|/mnt/Macintosh HD/Clarice Archive 20261117/||; s|/mnt/Macintosh HD/||' /tmp/archive-after.txt > /tmp/archive-after.paths || exit 1
sort /tmp/archive-after.paths > /tmp/archive-after.norm || exit 1
cmp /tmp/archive-before.norm /tmp/archive-after.norm || exit 1
cp /tmp/archive-after.txt "$a/verified-manifest.txt" || exit 1
echo ARCHIVE_VERIFIED
