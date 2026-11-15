show() { for value do printf '<%s>\n' "$value"; done; }
set -- 'a b' '' '*.txt'
show "$@"
IFS=:
printf '<%s>\n' "$*"
sh -c 'printf "%s:%s:%s\n" "$0" "$1" "$#"' worker 'two words'
x=$(false)
echo substitution:$?
x=$(true) y=$(false)
echo substitution:$?
