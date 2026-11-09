/* Builtin help is explicit; do not change echo/printf/test argument semantics. */
static int builtin_help(const char *topic){
 static const struct {const char *name,*text;} entries[]={
 {"cd","cd [directory]\nChange shell directory; default is the Clarice root. Paths may use /mnt.\n"},
 {"pwd","pwd\nPrint the full current shell path.\n"},
 {"echo","echo [word ...]\nPrint arguments separated by spaces and a newline; no option processing.\n"},
 {"printf","printf format [argument ...]\nFormats: %s, %d, %% and \\n, \\r, \\t. Other conversions are unsupported.\n"},
 {"export","export [NAME=value ...]\nSet or list exported variables; 16 entries, names 31 bytes, values 127 bytes.\n"},
 {"unset","unset name ...\nRemove variables. Function removal is not implemented.\n"},
 {"set","set [-- argument ...]\nList variables, or replace positional arguments. Shell option flags unsupported.\n"},
 {"shift","shift [count]\nDrop positional arguments; default one.\n"},
 {"read","read name\nRead one line into one variable (127-byte bound); no option or IFS splitting support.\n"},
 {"sh","sh file [argument ...] | sh -c command\nRun the bounded POSIX-inspired scripting subset; not full POSIX conformance.\n8 KiB source, 256 tokens, 16 nesting levels; pipelines use sequential /tmp spools.\n"},
 {".",". file [argument ...]\nSource a script in the current shell environment.\n"},
 {"test","test expression | [ expression ]\nSupported: -e -f -d -s -n -z, strings = !=, integers -eq -ne -lt -le -gt -ge, !.\nNo compound -a/-o expression parser.\n"},
 {"[","Use help test. A closing ] is required.\n"},
 {"true","true [argument ...]\nReturn success.\n"},{"false","false [argument ...]\nReturn status 1.\n"},{":",": [argument ...]\nNo operation; return success.\n"},
 {"break","break\nLeave the current loop; numeric levels unsupported.\n"},{"continue","continue\nStart the next iteration; numeric levels unsupported.\n"},
 {"return","return [status]\nReturn from a function or script.\n"},{"exit","exit [status]\nExit the script, or quit Clarice at the interactive prompt.\n"},
 {"history","history [on|off|clear]\nList or control the bounded disk-backed history in /etc/clarice/clarice.history.\nUp/Down traverse entries; history is separate from scrollback.\n"},
 {"prefs","prefs [window|fonts|font name|size 6..24|colours dark/light|scrollback 32/64/128|cache on/off|renderer native/c/asm|paging off/screen/100|columns N|fill]\nChanges persist in /etc/clarice/clarice.conf. Cache means terminal cache only.\n"},
 {"status","status [runtime]\nShow terminal/application metrics; runtime shows the last loaded command's memory/timing.\nNot Unix process accounting or total machine CPU usage.\n"},
 {"clear","clear\nClear the terminal display.\n"},
 {"selftest","selftest\nRun built-in ABI/parser/scan checks.\n"},{"checks","checks\nRun the application test set.\n"},{"scrollbench","scrollbench\nRun terminal rendering benchmarks.\n"},
 {"help","help [builtin]\nShow builtin usage. External utilities provide command --help.\n"}
 };
 for(unsigned i=0;i<sizeof entries/sizeof entries[0];i++)if(!strcmp(topic,entries[i].name)){say(entries[i].text);return 0;}
 errsay("help: unknown builtin; try command --help for an external utility\n");return 1;
}
static int help_command(int argc,char **av){if(argc>2)return 2;if(argc==2)return builtin_help(!strcmp(av[1],"--help")?"help":av[1]);say("Clarice: native System 6 command shell\nBuiltins: cd pwd echo printf export unset set shift read sh . test\n          true false : break continue return exit history prefs status clear\nUse help NAME for builtin details; command --help for external tools.\n/bin: ls df cat grep sed awk wc head tail cut tr sort uniq tee\n      cp mv touch mkdir rmdir rm ln readlink hfsattr\n      find du file od basename dirname more\nUp/Down: history; Tab: completion; Ctrl+C/Option-C/Cmd-period: interrupt.\nPOSIX sh is a compatibility target; current scripting/pipes are bounded.\nchmod is not implemented: hfsattr handles meaningful HFS metadata.\n");return 0;}
