appinstall replace /tmp/lfprobe.clx lfprobe
lfprobe
echo original=$?
/tmp/lftamper /bin/lfprobe
echo tamper=$?
lfprobe
echo fast=$?
CLX_VERIFY=strict lfprobe
echo strict=$?
lfprobe
echo revoked=$?
appinstall replace /tmp/lfprobe.clx lfprobe
lfprobe
echo repaired=$?
appinstall check /bin/lfprobe
echo checked=$?
