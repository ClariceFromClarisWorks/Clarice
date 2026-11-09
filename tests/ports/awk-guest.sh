echo AWK-START
status runtime
awk '{print $1, $2}' /tmp/awk-data
echo fields:$?
awk '{s += $2*$3} END {printf "%.2f\n",s}' /tmp/awk-data
echo sum:$?
awk 'BEGIN {FS="[ ]+";OFS=":"} {print $1,$2}' /tmp/awk-data
echo separator:$?
awk '{a[$1]+=$2} END {print a["apple"],a["pear"]}' /tmp/awk-data
echo arrays:$?
awk 'function twice(x){return 2*x} {print twice($2)}' /tmp/awk-data
echo functions:$?
awk 'BEGIN {print sqrt(2),2^3;printf "%d %04d\n",42,7}'
echo maths:$?
cat /tmp/awk-data | awk '$2>2 {print $1}' | grep pear
echo pipeline:$?
awk -f /tmp/awk-program /tmp/awk-data > /tmp/awk-sum
cat /tmp/awk-sum
echo program:$?
export AWK_TEST=present
awk 'BEGIN {print ENVIRON["AWK_TEST"]}'
echo environment:$?
awk 'BEGIN {print "written" > "/tmp/awk-written";close("/tmp/awk-written")}'
cat /tmp/awk-written
echo write:$?
awk 'BEGIN {while((getline x < "/tmp/awk-written")>0) print x}'
echo getline:$?
awk 'BEGIN {exit 7}'
echo exit:$?
awk 'BEGIN {' 2> /tmp/awk-error
echo syntax:$?
cat /tmp/awk-error
awk 'BEGIN {system("echo bad")}' 2> /tmp/awk-error
echo unsupported:$?
cat /tmp/awk-error
status runtime
for n in 1 2 3 4 5; do awk 'BEGIN {print 2+3}' > /tmp/awk-loop || echo FAIL; done
status runtime
echo AWK-END
