while IFS= read -r value; do printf '<%s>\n' "$value"; done <<EOF2 > result
one
two
EOF2
cat result
# Redirects survive function AST copies, including an inner subshell.
f() { ( echo copied ) > result; }
f
cat result
f
cat result
rm result
