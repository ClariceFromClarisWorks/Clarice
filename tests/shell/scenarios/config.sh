printf '%s\n' 'label="two words"' 'show() { printf "<%s>\\n" "$label"; }' > settings
. ./settings
show
first=one
second=old
unset first
second=new
unset second
printf '<%s>\n' "${second-default}"
x=outside
( x=inside; echo "$x" )
echo "$x"
rm settings
