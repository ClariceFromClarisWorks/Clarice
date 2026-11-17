count=0
for item in one skip two stop three; do
 case "$item" in skip) continue;; stop) break;; esac
 count=$((count+1))
 echo "$count:$item"
done
until [ "$count" -ge 4 ]; do count=$((count+1)); done
f() { set -- inner; printf '%s\n' "$1"; return 3; }
set -- 'outer value' end
f
rc=$?
printf '%s:%s:%s\n' "$rc" "$1" "$#"
[ "$count" -eq 4 ] && [ "$rc" -eq 3 ] || exit 8
