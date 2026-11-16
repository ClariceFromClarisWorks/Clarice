export PATH=/mnt/Audit/tools:/bin
cd /mnt/Audit/help
status runtime > d-before
{ echo discarded > /dev/null; } > d0.out 2> d0.err
echo $? > d0.status
{ echo visible; } > d1.out 2> d1.err
echo $? > d1.status
{ cksum /dev/null; } > d2.out 2> d2.err
echo $? > d2.status
{ cksum < /dev/null; } > d3.out 2> d3.err
echo $? > d3.status
{ cmp /dev/null /dev/null; } > d4.out 2> d4.err
echo $? > d4.status
{ od -A n -t x1 -N 32 < /dev/zero; } > d5.out 2> d5.err
echo $? > d5.status
{ grep x < /dev/null; } > d6.out 2> d6.err
echo $? > d6.status
{ { echo discarded; printf ignored; } > /dev/null; echo restored; } > d7.out 2> d7.err
echo $? > d7.status
{ cksum /dev/missing; } > d8.out 2> d8.err
echo $? > d8.status
{ cksum /dev/nullx; } > d9.out 2> d9.err
echo $? > d9.status
{ cd /dev; cksum ./null; cd /mnt/Audit/help; } > d10.out 2> d10.err
echo $? > d10.status
{ command -v ls; } > d11.out 2> d11.err
echo $? > d11.status
{ command -v read; } > d12.out 2> d12.err
echo $? > d12.status
{ f() { :; }; command -v f; } > d13.out 2> d13.err
echo $? > d13.status
{ command -v missing_command; } > d14.out 2> d14.err
echo $? > d14.status
{ command -v -- pwd; } > d15.out 2> d15.err
echo $? > d15.status
{ help command; } > d16.out 2> d16.err
echo $? > d16.status
{ free --help; } > d17.out 2> d17.err
echo $? > d17.status
{ free -b; } > d18.out 2> d18.err
echo $? > d18.status
{ free -h; } > d19.out 2> d19.err
echo $? > d19.status
{ free --bad; } > d20.out 2> d20.err
echo $? > d20.status
{ echo ignored 1>&2 2> /dev/null; } > d21.out 2> d21.err
echo $? > d21.status
{ cksum /dev/null > /dev/zero; } > d22.out 2> d22.err
echo $? > d22.status
i=0; while [ "$i" -lt 20 ]; do cksum /dev/null > /dev/null; cmp /dev/null /dev/null; i=$((i+1)); done
status runtime > d-after
echo DEVICES_DONE
