/mnt/Sys6Unix/Perf/pressure.clx /mnt/Sys6Unix/Perf/entry.clx
/mnt/Sys6Unix/Perf/pressure.clx /mnt/Sys6Unix/Perf/entry.clx fragmented
for x in 1 2 3; do
 time appinstall check /bin/awk
 /bin/cat /mnt/Sys6Unix/Perf/8192.sh >/dev/null; status runtime
 /bin/ls /tmp >/dev/null; status runtime
done
echo BEFORE-DONE
