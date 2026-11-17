# A small report with quoted paths, functions, loops and a pipeline.
mkdir -p "report data"
row() { printf '%s:%s\n' "$1" "$2"; }
{ row apple 3; row pear 2; row apple 4; } > "report data/input"
sed 's/:/ /' "report data/input" | awk '{s[$1]+=$2} END {for(k in s) print k,s[k]}' | sort > "report data/totals"
n=0
while IFS= read -r record; do
 n=$((n + 1))
 printf '%s=%s\n' "$n" "$record"
done < "report data/totals"
count=$(wc -l < "report data/totals")
[ "$count" -eq 2 ] || exit 9
cat "report data/totals"
rm -r "report data"
