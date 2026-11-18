: > /tmp/lo-results
for n in 100 1000; do
 for mode in names long; do
  opt=-a1
  test "$mode" = long && opt=-la
  for r in 0 1 2 3; do
   order="before after"
   test "$r" -eq 2 && order="after before"
   for v in $order; do
    echo file:$n:$mode:$r:$v >> /tmp/lo-results
    time /tmp/lo-$v/ls $opt /tmp/ls$n > /tmp/lo-out-$v 2>> /tmp/lo-results
    echo status:$? >> /tmp/lo-results
    status runtime >> /tmp/lo-results
   done
   cmp /tmp/lo-out-before /tmp/lo-out-after
   echo compare:$? >> /tmp/lo-results
  done
 done
done
for mode in names long; do
 opt=-a1
 test "$mode" = long && opt=-la
 for v in before after; do
  echo screen:100:$mode:0:$v >> /tmp/lo-results
  time /tmp/lo-$v/ls $opt /tmp/ls100 2>> /tmp/lo-results
  echo status:$? >> /tmp/lo-results
  status runtime >> /tmp/lo-results
 done
done
/tmp/ls-flush /tmp/lo-results
echo flush:$? >> /tmp/lo-results
/tmp/ls-flush /tmp/lo-results
echo OUTPUT-BENCHMARK-DONE
