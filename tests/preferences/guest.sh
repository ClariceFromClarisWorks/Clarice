echo PREFERENCES-CLI-START
prefs font Monaco
prefs size 12
prefs colours dark
prefs scrollback 32
prefs renderer c
prefs paging 100
prefs columns 40
cat /etc/clarice/clarice.conf > /tmp/prefs-cli.conf
prefs size 0
echo invalid-size:$?
prefs scrollback 200
echo invalid-scrollback:$?
prefs columns 999
echo invalid-columns:$?
prefs font NoSuchInstalledFont
echo missing-font:$?
prefs renderer asm
prefs fill
cat /etc/clarice/clarice.conf > /tmp/prefs-fill.conf
echo PREFERENCES-CLI-END
