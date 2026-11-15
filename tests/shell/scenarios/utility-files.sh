mkdir -p tree/sub
printf 'abc\n' > tree/source
touch tree/new
cp tree/source tree/sub/copy
mv tree/new tree/renamed
ln -s sub/copy tree/link
readlink tree/link
cat tree/link
find tree -type f | sort
basename tree/sub/copy
dirname tree/sub/copy
more tree/source > paged
wc -c < paged
od -A n -t x1 -N 4 tree/source | awk 'NF {$1=$1; print}'
if rmdir tree; then echo wrong; else echo nonempty; fi
rm tree/link tree/renamed tree/source tree/sub/copy
rmdir tree/sub tree
rm paged
