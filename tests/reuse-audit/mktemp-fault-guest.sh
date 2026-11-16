mkdir /tmp/mktemp-fault
/tmp/mktemp-write-fault -d /tmp/mktemp-fault/write.XXXXXX > /tmp/mktemp-fault/output
echo write:$? > /tmp/mktemp-fault/result
/tmp/mktemp-cancel-fault -d /tmp/mktemp-fault/cancel.XXXXXX > /tmp/mktemp-fault/output
# Returning 130 deliberately terminates this script. At the ordinary prompt,
# capture $? before running another command, then run a production mktemp -d,
# remove its returned directory and list the fixture directory. This checks
# shell recovery as well as command cleanup (see the recorded guest results).
