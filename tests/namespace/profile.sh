cat /etc/profile
cat > /etc/profile <<'END'
# Profile installed by namespace regression test.
export PATH=/usr/bin:/bin
export HOSTNAME=plus-test
export PROFILE_TEST=loaded
END
. /etc/profile
echo "$PATH" "$HOSTNAME" "$PROFILE_TEST"
hello profile
