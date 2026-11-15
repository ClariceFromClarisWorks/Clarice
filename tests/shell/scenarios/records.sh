printf '  apple  \npear\\tree\nlast' > records
n=0
while IFS= read -r value || [ -n "$value" ]; do
 n=$((n+1))
 printf '%s:<%s>\n' "$n" "$value"
done < records
printf 'a:b::c\n' | { IFS=: read -r value; printf '<%s>\n' "$value"; }
rm records
