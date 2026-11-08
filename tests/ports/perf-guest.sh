echo PERF-START
for n in 1 2 3; do grep -Fc alpha /tmp/grep-short; status runtime; done
for n in 1 2 3; do grep -c alpha /tmp/grep-short; status runtime; done
echo MEMORY-START
status runtime
for n in 1 2 3 4 5 6 7 8 9 10; do grep -q alpha /tmp/grep-input || echo FAIL; done
status runtime
for n in 1 2 3 4 5 6 7 8 9 10; do grep -q alpha /tmp/grep-input || echo FAIL; done
status runtime
echo PERF-END
