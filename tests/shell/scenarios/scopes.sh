# Nested scopes and control flow must always restore outer streams.
printf 'a\nb\n' > input
{ for outer in 1 2; do
    while IFS= read -r value; do
      case "$value" in a) continue;; esac
      echo "$outer:$value"
      break
    done < input
  done
} > result
cat result
f() { { echo function; return 7; } > inner; echo wrong; }
f
rc=$?
echo "return:$rc"
cat inner
( echo sub; echo err >&2 ) > subout 2> suberr
cat subout suberr
if { echo forbidden; } > missing/file; then echo wrong; else echo failure; fi
echo restored
rm input result inner subout suberr
