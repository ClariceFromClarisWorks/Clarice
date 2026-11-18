: > /tmp/lc-results
for n in 100 1000; do
 opt=-la
 test "$n" -eq 100 && opt=-lah
 for r in 0 1 2 3; do
  order="before after"
  test "$r" -eq 2 && order="after before"
  for v in $order; do
   echo file:$n:$r:$v >> /tmp/lc-results
   time /tmp/lc-$v/ls $opt /tmp/ls$n > /tmp/lc-out-$n-$v 2>> /tmp/lc-results
   echo status:$? >> /tmp/lc-results
   status runtime >> /tmp/lc-results
  done
 done
done
for opt in -a1 --full; do
 /tmp/lc-before/ls $opt /tmp/lc-samples > /tmp/lc-old
 /tmp/lc-after/ls $opt /tmp/lc-samples > /tmp/lc-new
 cmp /tmp/lc-old /tmp/lc-new
 echo unchanged:$? >> /tmp/lc-results
done
# screen:5 identifies the small fixture (4 children, plus . and ..).
for r in 0 1 2 3; do
 order="before after"
 test "$r" -eq 2 && order="after before"
 for v in $order; do
  echo screen:5:$r:$v >> /tmp/lc-results
  time /tmp/lc-$v/ls -lah /tmp/lc-samples 2>> /tmp/lc-results
  echo status:$? >> /tmp/lc-results
  status runtime >> /tmp/lc-results
 done
done
/tmp/lc-after/ls --legend -lah /tmp/lc-samples > /tmp/lc-legend
 echo legend:$? >> /tmp/lc-results
/tmp/lc-after/ls --legend --full /tmp/lc-samples > /tmp/lc-full
 echo full:$? >> /tmp/lc-results
/tmp/lc-after/ls --legend --wrong > /tmp/lc-bad 2> /tmp/lc-error
 echo invalid:$? >> /tmp/lc-results
/tmp/lc-after/ls --legend -lah /tmp/lc-samples
/tmp/ls-flush /tmp/lc-results
echo LS-CODES-DONE
