BEGIN {
 for (i=1;i<=16;i++) {p="/mnt/Audit/awk-out-" i; print i > p}
 for (i=1;i<=16;i++) {p="/mnt/Audit/awk-out-" i; if(close(p)) exit 2}
 print "AWK16_OK"
}
