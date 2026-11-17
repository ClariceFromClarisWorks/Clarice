export PATH=/mnt/Audit/tools:/bin
cd /mnt/Audit/help
status runtime > second-before
{ env -i; } > b0.out 2> b0.err
echo $? > b0.status
{ export A=parent; env -i A=child sh -c 'echo $A'; echo $A; } > b1.out 2> b1.err
echo $? > b1.status
{ export HOME=/custom; printenv HOME; } > b2.out 2> b2.err
echo $? > b2.status
{ env -i /mnt/Audit/tools/printenv HOME; echo $?; } > b3.out 2> b3.err
echo $? > b3.status
{ env -i HOME=/only /mnt/Audit/tools/printenv; } > b4.out 2> b4.err
echo $? > b4.status
{ env -u HOME printenv HOME; echo $?; } > b5.out 2> b5.err
echo $? > b5.status
{ time false; echo $?; } > b6.out 2> b6.err
echo $? > b6.status
{ env -i exit 7; echo alive; } > b7.out 2> b7.err
echo $? > b7.status
{ sleep 0; } > b8.out 2> b8.err
echo $? > b8.status
{ sleep --bad; } > b9.out 2> b9.err
echo $? > b9.status
{ printenv --bad; } > b10.out 2> b10.err
echo $? > b10.status
{ dd if=/mnt/Audit/help/binary of=/mnt/Audit/help/copy bs=1024; cmp /mnt/Audit/help/binary /mnt/Audit/help/copy; } > b11.out 2> b11.err
echo $? > b11.status
{ dd if=/dev/zero of=/mnt/Audit/help/zeros bs=1024 count=8; cksum /mnt/Audit/help/zeros; } > b12.out 2> b12.err
echo $? > b12.status
{ dd if=/mnt/Audit/help/binary bs=137 count=4 | cksum; } > b13.out 2> b13.err
echo $? > b13.status
{ printf keep > keep; dd if=/dev/zero of=keep bs=2147483648; cat keep; } > b14.out 2> b14.err
echo $? > b14.status
{ dd if=/mnt/Audit/help/binary of=seek ibs=7 obs=13 seek=2 count=5; cksum seek; } > b15.out 2> b15.err
echo $? > b15.status
{ printf prefix > gap; dd if=/mnt/Audit/help/binary of=gap bs=31 count=1 seek=10 conv=notrunc; cksum gap; } > b16.out 2> b16.err
echo $? > b16.status
{ dd if=/dev/zero of=empty count=0; cksum empty; } > b17.out 2> b17.err
echo $? > b17.status
{ p=$(mktemp /mnt/Audit/help/owned.XXXXXX); test -f "$p"; echo created:$?; rm "$p"; test -f "$p"; echo removed:$?; } > b18.out 2> b18.err
echo $? > b18.status
{ TMPDIR=/mnt/Audit/help; export TMPDIR; p=$(mktemp); echo "$p"; rm "$p"; } > b19.out 2> b19.err
echo $? > b19.status
{ mktemp -u; } > b20.out 2> b20.err
echo $? > b20.status
{ mktemp -d; } > b21.out 2> b21.err
echo $? > b21.status
{ mktemp -q /mnt/no-volume/tmp.XXXXXX; } > b22.out 2> b22.err
echo $? > b22.status
{ dd if=/mnt/no-volume/missing of=absent; } > b23.out 2> b23.err
echo $? > b23.status
{ dd conv=noerror; } > b24.out 2> b24.err
echo $? > b24.status
{ dd --bad; } > b25.out 2> b25.err
echo $? > b25.status
{ env -i A=one /mnt/Audit/tools/printenv A | grep one; } > b26.out 2> b26.err
echo $? > b26.status
{ time -- false; } > b27.out 2> b27.err
echo $? > b27.status
{ printenv MISSING_CLARICE_VALUE; } > b28.out 2> b28.err
echo $? > b28.status
{ export HOME=/large; awk 'BEGIN { print ENVIRON["HOME"] }'; } > b29.out 2> b29.err
echo $? > b29.status
{ env -i A=one /mnt/Audit/tools/awk 'BEGIN { print ENVIRON["A"]; print ENVIRON["HOME"] }'; } > b30.out 2> b30.err
echo $? > b30.status
{ printf safe > oomkeep; dd if=/dev/zero of=oomkeep bs=1048576; echo failed:$?; cat oomkeep; } > b31.out 2> b31.err
echo $? > b31.status
{ printf locked > locked; hfsattr --locked on locked; dd if=/dev/zero of=locked bs=16 count=1; echo failed:$?; hfsattr --locked off locked; cat locked; } > b32.out 2> b32.err
echo $? > b32.status
{ dd if=/dev/zero of=/dev/null bs=1024 count=16; status runtime; } > b33.out 2> b33.err
echo $? > b33.status
{ printenv PATH > /dev/null; status runtime; } > b34.out 2> b34.err
echo $? > b34.status
{ p=$(mktemp /mnt/Audit/help/peak.XXXXXX); status runtime; rm "$p"; } > b35.out 2> b35.err
echo $? > b35.status
{ export BIG='xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'; env -i BIG="$BIG" /mnt/Audit/tools/printenv BIG | wc -c; unset BIG; } > b36.out 2> b36.err
echo $? > b36.status
for i in 1 2 3 4 5 6; do dd if=/dev/zero of=/dev/null bs=1024 count=16; printenv PATH > /dev/null; p=$(mktemp /mnt/Audit/help/repeat.XXXXXX); rm "$p"; status runtime; done > second-repeat 2> second-repeat.err
status runtime > second-after
echo SECOND_DONE
