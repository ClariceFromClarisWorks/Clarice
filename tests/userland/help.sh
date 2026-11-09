echo HELP-START
for tool in ls df cat grep sed awk wc head tail cut tr sort uniq tee cp mv touch mkdir rmdir rm ln readlink hfsattr find du file od basename dirname more; do $tool --help > /tmp/ul-help; echo $tool:$?; done
for tool in cd pwd echo printf export unset set shift read sh . test true false break continue return exit history prefs status clear; do help $tool > /tmp/ul-help; echo builtin:$tool:$?; done
help unknown
echo unknown:$?
echo HELP-END
