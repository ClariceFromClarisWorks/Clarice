echo HFSATTRIB-BEGIN
command -v hfsattrib
command -v hfsattr
echo old-status=$?
hfsattrib --help
hfsattrib /tmp/attrib
hfsattrib /tmp/attrib SIT!:SITx
echo shorthand=$?
hfsattrib /tmp/attrib
hfsattrib /tmp/attrib sit!:sitX
hfsattrib /tmp/attrib
hfsattrib /tmp/attrib SIT!:BAD
echo invalid=$?
hfsattrib /tmp/attrib
hfsattrib --type BAD --creator TEST /tmp/attrib
echo invalid-option=$?
hfsattrib --locked on /tmp/attrib SIT!:BAD
echo invalid-mixed=$?
hfsattrib --type TEXT /tmp/attrib
hfsattrib --creator TEST /tmp/attrib
hfsattrib --invisible on /tmp/attrib
hfsattrib /tmp/attrib
hfsattrib --invisible off /tmp/attrib
hfsattrib --locked on /tmp/attrib
hfsattrib /tmp/attrib
hfsattrib --locked off /tmp/attrib
hfsattrib /tmp /tmp/attrib
hfsattrib /tmp SIT!:SITx
echo directory-status=$?
hfsattrib /tmp/attrib SIT!:SITx
status runtime
for i in 1 2 3 4 5 6 7 8 9 10; do hfsattrib /tmp/attrib SIT!:SITx; done
status runtime
for i in 1 2 3 4 5 6 7 8 9 10; do hfsattrib /tmp/attrib SIT!:SITx; done
status runtime
hfsattrib /tmp/attrib
echo HFSATTRIB-END
