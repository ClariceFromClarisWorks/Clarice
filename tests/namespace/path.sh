echo "$PATH"
cat > /bin/hello <<'END'
#!/bin/sh
echo bin "$1"
END
cat > /usr/bin/hello <<'END'
#!/bin/sh
echo usr "$1"
END
hello first
PATH=/usr/bin:/bin
hello second
ln -s /bin/hello /bin/linked
PATH=/bin:/usr/bin
linked link
cat > /tmp/local.sh <<'END'
echo local
END
cd /tmp
local.sh
echo status:$?
./local.sh
PATH=
local.sh
PATH=/does-not-exist
pwd
hello
echo status:$?
hello() { echo function; }
hello
