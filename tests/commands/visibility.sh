ls -1 /tmp/visibility > /tmp/visible-only.txt
ls -la /tmp/visibility > /tmp/visible-all.txt
ls -la --full /tmp/visibility > /tmp/visible-full.txt
ls -l /tmp/visibility/invisible > /tmp/visible-explicit.txt
ls --help > /tmp/ls-help.txt
echo VISIBILITY_COMPLETE
