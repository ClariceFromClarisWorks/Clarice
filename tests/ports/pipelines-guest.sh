echo PIPELINES-START
wc /tmp/pipeline-data
echo wc:$?
head -n 2 /tmp/pipeline-data
echo head:$?
cut -f 2 -d ' ' /tmp/pipeline-data
echo cut:$?
tr a-z A-Z < /tmp/pipeline-data
echo tr:$?
uniq -c /tmp/pipeline-data
echo uniq:$?
cat /tmp/pipeline-data | cut -f 2 -d ' ' | uniq | wc -l
echo pipe:$?
head -n 2 /tmp/pipeline-data > /tmp/pipeline-first
wc -c < /tmp/pipeline-first
echo redirect:$?
head -n bad /tmp/pipeline-data 2> /tmp/pipeline-error
echo invalid:$?
wc /tmp/no-such-pipeline-file 2> /tmp/pipeline-error
echo missing:$?
head /tmp/pipeline-long 2> /tmp/pipeline-error
echo bound:$?
for tool in wc head cut tr uniq; do $tool --help > /tmp/pipeline-help; echo help:$tool:$?; done
status runtime
for n in 1 2 3 4 5; do wc /tmp/pipeline-data > /tmp/pipeline-out; head -n 2 /tmp/pipeline-data > /tmp/pipeline-out; cut -b 1-3 /tmp/pipeline-data > /tmp/pipeline-out; tr a-z A-Z < /tmp/pipeline-data > /tmp/pipeline-out; uniq -c /tmp/pipeline-data > /tmp/pipeline-out; done
status runtime
echo PIPELINES-END
