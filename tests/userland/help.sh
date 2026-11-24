echo HELP-START
for tool in ls df cat benchls benchscan grep sed awk serial wc wc-sbase head cut tr tr-sbase uniq tee tail sort sort-sbase cp mv touch rmdir mkdir rm ln readlink hfsattrib find du file od more basename dirname kermit wget cmp cksum free sleep printenv mktemp dd ipconfig ping ftp telnet vi; do
    echo COMMAND $tool
    $tool --help
    echo RESULT $?
done
for tool in cd pwd echo printf export unset set shift read sh . test '[' true false : break continue return exit env time command history prefs status clear appinstall remote clarice selftest checks scrollbench help; do
    echo BUILTIN $tool
    help "$tool"
    echo RESULT $?
done
for tool in help env time appinstall clarice; do
    echo ENTRY $tool
    $tool --help
    echo RESULT $?
done
echo INDEX
help
echo RESULT $?
help unknown
echo UNKNOWN $?
help cd pwd
echo EXCESS $?
echo HELP-END
