name=reader
cat <<EOF2 > letter
hello $name
EOF2
cat <<'EOF2' >> letter
literal $name
EOF2
cat letter
rm letter
printf 'after\n'
