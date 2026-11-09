echo MORE-PIPELINES-START
tail -n 2 /tmp/pipeline-data
echo tail:$?
tail -c 6 /tmp/pipeline-data
tail -n +2 /tmp/pipeline-data
cat /tmp/pipeline-data | tee /tmp/ul-tee | wc -l
echo tee:$?
echo appended | tee -a /tmp/ul-tee
wc -l /tmp/ul-tee
sort /tmp/ul-sort > /tmp/ul-sorted
echo sort:$?
sort -c /tmp/ul-sorted
echo sorted:$?
head -n 2 /tmp/ul-sorted
tail -n 2 /tmp/ul-sorted
sort -n /tmp/ul-numbers
sort -u /tmp/pipeline-data
status runtime
for n in 1 2 3; do sort /tmp/pipeline-data > /tmp/ul-out; tail -n 1 /tmp/pipeline-data > /tmp/ul-out; head -n 1 /tmp/pipeline-data | tee /tmp/ul-out > /tmp/ul-out2; done
status runtime
echo MORE-PIPELINES-END
