{ printf 'alpha\nbeta\ngamma\n'; } | while IFS= read -r value; do
 case "$value" in beta) echo "$value";; esac
done > result
cat result
{ echo first; echo second >&2; } > combined 2>&1
cat combined
# Reverse order keeps stderr on the prior stdout.
{ echo redirected; echo visible >&2; } 2>&1 > separated
cat separated
for x in a b; do echo "$x"; done >> result
cat result
if false; then echo bad; else echo good; fi > conditional
case x in x) echo case;; esac >> conditional
cat conditional
rm result combined separated conditional
