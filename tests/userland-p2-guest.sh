mkdir -p /tmp/p2
printf 'HOME=<%s> tilde=<%s>\n' "$HOME" ~
cd; pwd; cd /
HOME=/tmp sh -c 'cd; pwd; printf "override=<%s>\n" ~'
printf 'restored=<%s>\n' "$HOME"
for x in a b; do echo "$x" | cat; break | cat; echo kept; done
exit 7 | cat; echo pipeline-alive
false; printf 'sub=<%s> status=%s\n' "$(true)" "$?"
true; value=$(false); echo assignment-status=$?
IFS=:; set -- a b 'c:d' ''; printf 'at=<%s>\n' $@
IFS=; set -- a b ''; printf 'empty-ifs=<%s>\n' pre$@post
unset IFS
p=; for x in 1 2 3 4 5 6 7 8 9 10; do p=${p}aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa; done
PATH=$p:/bin; command -v cat; printf 'path-ok\n' | cat
for n in 1 2 3 4 5; do
 /mnt/Sys6Unix/P2/device.clx 0 </dev/null >/dev/null 2>/dev/null
 echo device-status=$?
 /mnt/Sys6Unix/P2/device.clx 0 </dev/zero >/dev/null 2>/dev/zero
 echo zero-status=$?; status runtime
done
echo P2-LOCAL-DONE
