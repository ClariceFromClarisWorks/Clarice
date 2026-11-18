mv /tmp/ls-fmt /tmp/ls-frmt
: > /tmp/ls-results
# Disposable System 6 guest. Install base/fmt/next packages as documented.
# Record command-loading/execution time, not final-prompt presentation.
for n in 100 1000; do
 for r in 0 1 2 3; do
  order="base frmt next"
  test "$r" -eq 2 && order="next frmt base"
  for v in $order; do
   echo file:$n:$r:$v >> /tmp/ls-results
   time /tmp/ls-$v/ls -la /tmp/ls$n > /tmp/out-$v 2>> /tmp/ls-results
   echo status:$? >> /tmp/ls-results
   status runtime >> /tmp/ls-results
  done
  cmp /tmp/out-base /tmp/out-frmt; echo compare-fmt:$? >> /tmp/ls-results
  cmp /tmp/out-base /tmp/out-next; echo compare-next:$? >> /tmp/ls-results
 done
done
echo LISTING-FILE-DONE

# Same fixtures/packages after guest.sh; paint/output to the local terminal.
for n in 100 1000; do
 for r in 0 1 2; do
  order="base frmt next"
  test "$r" -eq 1 && order="next frmt base"
  for v in $order; do
   echo screen:$n:$r:$v >> /tmp/ls-results
   time /tmp/ls-$v/ls -la /tmp/ls$n 2>> /tmp/ls-results
   echo status:$? >> /tmp/ls-results
   status runtime >> /tmp/ls-results
  done
 done
done
echo LISTING-SCREEN-DONE

# Target the parent-lookup saving separately, without decimal formatting.
for r in 0 1 2; do
 order="base next"
 test "$r" -eq 1 && order="next base"
 for v in $order; do
  echo names:100:$r:$v >> /tmp/ls-results
  time /tmp/ls-$v/ls -a1 /tmp/ls100 > /tmp/names-$v 2>> /tmp/ls-results
  echo status:$? >> /tmp/ls-results
  status runtime >> /tmp/ls-results
 done
 cmp /tmp/names-base /tmp/names-next
done
echo LISTING-NAMES-DONE

/tmp/ls-flush /
echo flush:$? >> /tmp/ls-results
/tmp/ls-flush / && echo ALL-LISTING-TESTS-DONE-FLUSHED
