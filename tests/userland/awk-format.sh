echo AWK-FORMAT-START
awk 'BEGIN {printf "%d %04d %x %u\n",42,7,255,4294967295}'
awk 'BEGIN {printf "%+08d %.0d %#o\n",-42,0,63}'
awk 'BEGIN {print sqrt(2), 2^3; printf "%.2f\n", 1.25}'
echo awk:$?
echo AWK-FORMAT-END
