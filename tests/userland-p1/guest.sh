mkdir -p /tmp/p1
free > /tmp/p1/memory
X=temporary true
X=temporary echo frame-ok
( X=nested true; echo nested-ok )
case abc in \*) echo FAIL-escaped;; *) echo PASS-escaped;; esac
case '\' in [\*]) echo FAIL-bracket;; *) echo PASS-bracket;; esac
x='*abc'; printf 'trim=%s\n' "${x#\*}"
touch '/tmp/p1/a b' /tmp/p1/abc /tmp/p1/axc
printf 'glob=<%s>\n' /tmp/p1/a*c
printf 'literal=<%s>\n' /tmp/p1/\*
for n in 1 2 3 4 5; do /mnt/Sys6Unix/P1/append.clx /tmp/p1/append; echo append-status=$?; status runtime; done
cat /tmp/p1/append
free >> /tmp/p1/memory
echo P1-LOCAL-DONE
