p='/mnt/Macintosh HD'
a="$p/Clarice Archive 20261117"
# Only identified earlier development copies; never current/nearest recoveries.
active=$(/tmp/id50) || exit 1
test "$active" = 'Macintosh HD:.cu-0000039b-78c4021b.old' || exit 1
test -d "$a" || mkdir "$a" || exit 1
for s in "$p"/.cu-*.old "$p/Clarice" "$p/Clarice Update" "$p/Clarice UpdatstClariceuste" "$p/Clarice Update.bin" "$p/Update2.bin"; do
  case "$s" in
    "$p/.cu-0000039b-78c4021b.old"|"$p/.cu-0000065a-78c402e0.old"|"$p/.cu-0000065d-78c402e1.old") continue ;;
  esac
  test -f "$s" || continue
  n=${s##*/}
  test ! -e "$a/$n" || exit 1
  before=$(file -b "$s") || exit 1
  flags=$(hfsattr "$s" | sed 's/^.*: //') || exit 1
  dates=$(ls -l "$s" | awk '{print $1,$2,$3,$4,$5}') || exit 1
  mv "$s" "$a/$n" || exit 1
  after=$(file -b "$a/$n") || exit 1
  flags2=$(hfsattr "$a/$n" | sed 's/^.*: //') || exit 1
  dates2=$(ls -l "$a/$n" | awk '{print $1,$2,$3,$4,$5}') || exit 1
  test "$before" = "$after" || exit 1
  test "$flags" = "$flags2" || exit 1
  test "$dates" = "$dates2" || exit 1
  test ! -e "$s" || exit 1
  echo "ARCHIVED $n"
  echo "$n: $after | $flags2 | $dates2" >> "$a/manifest.txt"
done
echo ARCHIVE_VERIFIED
