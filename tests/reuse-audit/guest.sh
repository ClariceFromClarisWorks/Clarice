mkdir /tmp/reuse-fixture
mkdir /tmp/reuse-fixture/sub
touch /tmp/reuse-fixture/file1.txt /tmp/reuse-fixture/file2.c /tmp/reuse-fixture/alpha.txt /tmp/reuse-fixture/9beta.log '/tmp/reuse-fixture/literal[.txt' '/tmp/reuse-fixture/a*.txt' /tmp/reuse-fixture/sub/file23.txt
/tmp/reuse-after/find /tmp/reuse-fixture -name 'file[0-9]*.txt' > /tmp/reuse-range
 echo $? > /tmp/reuse-status
/tmp/reuse-after/find /tmp/reuse-fixture -name '[!0-9]*' -type f > /tmp/reuse-neg
/tmp/reuse-after/find /tmp/reuse-fixture -name '[[:digit:]]*' > /tmp/reuse-class
/tmp/reuse-after/find /tmp/reuse-fixture -name 'a\*.txt' > /tmp/reuse-escape
/tmp/reuse-after/find /tmp/reuse-fixture -name 'literal[.txt' > /tmp/reuse-literal
/tmp/reuse-after/find /missing > /tmp/reuse-missing 2> /tmp/reuse-error
echo $? >> /tmp/reuse-status
for dir in /tmp/reuse-before /tmp/reuse-after; do
 for n in 1 2 3; do
  $dir/find /tmp/reuse-fixture -name '*.txt' > /tmp/reuse-common
  echo $? >> /tmp/reuse-status
  status runtime >> /tmp/reuse-timing
 done
 for n in 1 2 3; do
  $dir/find /mnt/Sys6Unix/bench/n1000 -name 'file*' > /dev/null
  echo $? >> /tmp/reuse-status
  status runtime >> /tmp/reuse-timing
 done
done
/tmp/reuse-after/find --help > /tmp/reuse-help
echo REUSE_DONE
