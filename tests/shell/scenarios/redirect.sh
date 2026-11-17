printf 'ok\n' > output
sh -c 'echo error >&2; exit 4' 2> errors
rc=$?
echo "status:$rc"
cat output errors
sh -c 'echo first; echo second >&2' > both 2>&1
cat both
# A redirection failure must prevent the command body.
if echo bad > absent/file; then echo wrong; else echo failed; fi
rm output errors both
