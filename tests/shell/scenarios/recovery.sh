# Failure status must drive recovery, not appear as success.
check() { grep -q "$1" "$2"; }
printf '%s\n' alpha beta > input
if check gamma input; then exit 7; else echo absent:$?; fi
if check beta input; then echo found; fi
false | true
echo pipeline:$?
true | false
echo pipeline:$?
message=$(printf '%s\n\n' 'two words')
printf '<%s>\n' "$message"
{ printf 'first\n'; printf 'second\n'; } > output
printf 'third\n' >> output
cat output | head -n 2 | tail -n 1
rm input output
