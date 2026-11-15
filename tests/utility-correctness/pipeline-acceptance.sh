printf 'b 2\na 1\na 1\nc 3\n' > pipe-input
sort pipe-input | uniq | tee pipe-sorted | cut -d ' ' -f 1 | tr a-z A-Z > pipe-result
cat pipe-result
wc -l < pipe-sorted
head -n 2 pipe-sorted | tail -n 1
awk '{sum+=$2} END {print sum}' pipe-sorted
sed -n '2p' pipe-sorted
grep x missing-file 2> /mnt/Audit/negative.err
echo grep:$?
sed p missing-file 2> /mnt/Audit/negative.err
echo sed:$?
awk 1 missing-file 2> /mnt/Audit/negative.err
echo awk:$?
cut -b 1 missing-file 2> /mnt/Audit/negative.err
echo cut:$?
for tool in wc head tail sort uniq; do
 $tool missing-file > /mnt/Audit/negative.out 2> /mnt/Audit/negative.err
 echo "$tool:$?"
done
rm pipe-input pipe-sorted pipe-result
