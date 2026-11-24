/* Explicit topics preserve argument-sensitive builtins such as echo and test. */
#include "help-output.h"

static int builtin_help(const char *topic){
 static const struct {const char *name,*text;} entries[]={
 {"env",
  "Usage:\n"
  "  env [-i] [-u NAME] [--] [NAME=VALUE ...] [command [arg ...]]\n"
  "Run a command with scoped exports, or print NAME=value entries. Parent shell state is restored.\n"
  "Options:\n"
  "  -i, -\tStart with an empty environment.\n"
  "  -u NAME\tRemove a shell identifier from the environment.\n"
  "Assignments follow options. Resolves builtins and commands without reparsing or starting Unix processes.\n"
  "Example:\n"
  "  env -i PATH=/bin sh -c 'echo ready'\n"
  "Status: command status, 0 printed, 2 usage/allocation failure.\n"},
 {"time",
  "Usage:\n"
  "  time [--] command [argument ...]\n"
  "Run a command and print elapsed wall time to stderr, in seconds.\n"
  "Native 60 Hz ticks; includes command loading and execution.\n"
  "No kernel CPU accounting: user/sys and -p are not fabricated.\n"
  "Times one command, not an entire surrounding pipeline.\n"
  "Parent shell state is restored; status is the command's status.\n"
  "Example:\n"
  "  time ls /mnt\n"},
 {"command",
  "Usage:\n"
  "  command -v [--] name\n"
  "Report a builtin, function, reserved word or executable PATH result.\n"
  "Uses normal Clarice CLX/script resolution; does not execute the command.\n"
  "One name per call. No -p/-V or command-execution form yet.\n"
  "Status: 0 found (or no name), 127 missing, 2 usage.\n"
  "Example:\n"
  "  command -v ls\n"},
 {"clarice",
  "Usage:\n"
  "  clarice update [-y] [--check] [--source PATH_OR_HTTP_URL]\n"
  "Check or install standard tools and Core. Keeps newer tools and prepares a native replacement helper automatically.\n"
  "See clarice --help for source, option and recovery details. Updates require local input.\n"},
 {"appinstall",
  "Usage:\n"
  "  appinstall setup DIRECTORY\n"
  "  appinstall install FILE NAME\n"
  "  appinstall check FILE\n"
  "  appinstall replace FILE NAME\n"
  "  appinstall repair NAME DIRECTORY\n"
  "  appinstall recover NAME\n"
  "Install or verify commands in /bin. NAME uses 1 to 20 letters, digits, _ or -.\n"
  "Actions:\n"
  "  setup\tInstall the offline standard collection.\n"
  "  install\tInstall one file. Like setup, keep existing files, even if damaged.\n"
  "  check\tVerify a file without executing it.\n"
  "  replace\tReplace explicitly, retaining a backup.\n"
  "  repair\tReplace with DIRECTORY/NAME.\n"
  "  recover\tRestore .clx-NAME.old.\n"
  "Backups keep both forks. Replacement preserves Finder metadata; the new CLX is data-only.\n"
  "Existing .old/.new files block replacement. Inspect and move them first.\n"
  "CLX_VERIFY=strict verifies on each launch. The installed mode uses receipts.\n"
  "Status: 0 success, 1 install error, 2 usage, 126 bad CLX, 130 cancelled.\n"},
 {"cd",
  "Usage:\n"
  "  cd [directory]\n"
  "Change shell directory; default is the Clarice root. Paths may use /mnt.\n"},
 {"pwd",
  "Usage:\n"
  "  pwd\n"
  "Print the full current shell path.\n"},
 {"echo",
  "Usage:\n"
  "  echo [word ...]\n"
  "Print arguments separated by spaces and a newline; no option processing.\n"},
 {"printf",
  "Usage:\n"
  "  printf [--] format [argument ...]\n"
  "Format arguments to stdout. Repeat the format while arguments remain.\n"
  "Conversions: %s %b %c %d %i %u %o %x %X and %%.\n"
  "Supports literal width/precision and integer flags - + space 0 #. Integers are 32-bit. No floating conversions or dynamic * width/precision.\n"
  "Missing arguments give empty strings or zero. Numbers accept base prefixes and a quoted character.\n"
  "%b decodes escapes; its \\c stops all output.\n"
  "Escapes: \\a \\b \\f \\n \\r \\t \\v \\\\ and octal. Binary NUL is preserved.\n"
  "Example:\n"
  "  printf '%s\\n' \"$@\"\n"
  "Status: 0 success, 1 bad number/output error, 2 bad format, 130 cancelled.\n"},
 {"export",
  "Usage:\n"
  "  export [-p]\n"
  "  export NAME[=VALUE] ...\n"
  "Pass variables to child commands. An assignment also sets the value.\n"
  "Options:\n"
  "  -p\tPrint shell-quoted exports, including unset names. This is also the default with no operands.\n"
  "Private variables stay in the shell. Names, values and entries grow with available memory.\n"
  "Example:\n"
  "  export PATH=/bin:/usr/bin\n"
  "Status: 0 success, 2 invalid name/allocation failure. See help unset.\n"},
 {"unset",
  "Usage:\n"
  "  unset [-v|-f] NAME ...\n"
  "Remove variables or functions. An absent name succeeds.\n"
  "Options:\n"
  "  -v\tRemove variables (default).\n"
  "  -f\tRemove functions.\n"
  "There are no readonly variables. See help export and help set.\n"},
 {"set",
  "Usage:\n"
  "  set\n"
  "  set [--|-] argument ...\n"
  "Without operands list variables. Otherwise replace $1, $2, etc., keeping $0.\n"
  "Use -- before operands starting with - or +. set -- clears arguments. A lone set - preserves them.\n"
  "Storage grows with available memory. Shell-option flags such as -e, -u and -f are unsupported.\n"
  "Status: 0 success, 2 unsupported option or allocation failure.\n"},
 {"shift",
  "Usage:\n"
  "  shift [count]\n"
  "Drop positional arguments; default one.\n"},
 {"read",
  "Usage:\n"
  "  read [-r] [--] NAME ...\n"
  "Read one stdin record. Split using IFS and assign fields in order. The last variable receives remaining fields.\n"
  "Options:\n"
  "  -r\tKeep backslashes. Normally a backslash quotes the next byte or joins an escaped newline.\n"
  "LF and native CR terminate records. Records grow with memory. NUL is rejected.\n"
  "Allocation failure preserves previous values. EOF assigns collected input, then returns nonzero.\n"
  "Example:\n"
  "  IFS= read -r line < file\n"
  "Status: 0 record read, 1 EOF, 2 error, 130 cancelled.\n"},
 {"sh",
  "Usage:\n"
  "  sh file [argument ...]\n"
  "  sh -c command [name [argument ...]]\n"
  "Run a script in a private shell environment. With -c, name becomes $0.\n"
  "Exported values are inherited. Private variables and functions stay in the parent.\n"
  "Quotes, expansion, functions, loops, redirection and finite pipelines are supported. Pipes spool sequentially to /tmp.\n"
  "Source, tokens and functions grow with memory; recursion is bounded. No jobs, traps or Bash syntax.\n"
  "Status: last command or explicit exit status, 2 error, 130 cancelled.\n"
  "See help . to run a script in the current environment.\n"},
 {".",
  "Usage:\n"
  "  . file [argument ...]\n"
  "Read a script in the current shell. Variable and function changes persist.\n"
  "With no arguments, positional changes also persist. Explicit arguments are scoped to the script.\n"
  "Supply a file path; the shell does not search PATH.\n"
  "Status: script result, 2 read/parse error, 130 cancelled. See help sh.\n"},
 {"test",
  "Usage:\n"
  "  test expression | [ expression ]\n"
  "Return 0 for true, 1 for false, 2 for invalid expressions.\n"
  "Files: -e exists, -f nondirectory, -d directory, -s nonempty data fork,\n"
  "-L/-h Clarice symbolic link. Follow links for other file tests.\n"
  "Strings: -n nonempty, -z empty, left = right, left != right.\n"
  "Integers: -eq -ne -lt -le -gt -ge (signed native 32-bit decimal).\n"
  "! negates; quoted parentheses group a supported expression.\n"
  "Use shell && and ||, not legacy -a/-o. Quote expansions with spaces.\n"
  "No Unix ownership or access-permission tests on HFS.\n"
  "Example:\n"
  "  test -f \"$path\" && printf '%s\\n' \"$path\"\n"},
 {"[",
  "Usage:\n"
  "  [ expression ]\n"
  "The test builtin with a required separate closing ] operand.\n"
  "Use help test for all operators, statuses and HFS differences.\n"},
 {"true",
  "Usage:\n"
  "  true [argument ...]\n"
  "Return success.\n"},
 {"false",
  "Usage:\n"
  "  false [argument ...]\n"
  "Return status 1.\n"},
 {":",
  "Usage:\n"
  "  : [argument ...]\n"
  "No operation; return success.\n"},
 {"break",
  "Usage:\n"
  "  break\n"
  "Leave the current loop; numeric levels unsupported.\n"},
 {"continue",
  "Usage:\n"
  "  continue\n"
  "Start the next iteration; numeric levels unsupported.\n"},
 {"return",
  "Usage:\n"
  "  return [status]\n"
  "Return from a function or script.\n"},
 {"exit",
  "Usage:\n"
  "  exit [status]\n"
  "Exit the script, or quit Clarice at the interactive prompt.\n"},
 {"history",
  "Usage:\n"
  "  history [on|off|clear]\n"
  "List or control bounded command history in /etc/clarice/clarice.history.\n"
  "Up/Down recall commands. Use the scrollbar to review terminal output.\n"},
 {"prefs",
  "Usage:\n"
  "  prefs [setting [value]]\n"
  "Show or change terminal preferences. With no arguments, show settings.\n"
  "Settings:\n"
  "  window\tOpen Preferences.\n"
  "  fonts\tList installed fonts.\n"
  "  font NAME\tSelect a font; quote names with spaces.\n"
  "  size N\tSet 6 to 24 points.\n"
  "  colours dark|light\tApply a theme immediately.\n"
  "  fill\tToggle full-screen.\n"
  "  control KEY\tChoose control, option (default) or command as the Control substitute.\n"
  "  scrollback N\tRetain 32, 64 or 128 lines. Reducing it discards older lines.\n"
  "  columns N, rows N\tSet a positive grid size, clamped to the screen and font.\n"
  "Changes persist in /etc/clarice/clarice.conf.\n"
  "Physical Control always works. In Command mode, add Option for shortcut conflicts. Command-period cancels locally.\n"
  "Status: 0 success, 2 invalid setting.\n"},
 {"status",
  "Usage:\n"
  "  status [-h]\n"
  "  status runtime\n"
  "Show terminal and application metrics.\n"
  "Options:\n"
  "  -h\tAccepted for compatibility. Sizes are readable by default.\n"
  "  runtime\tReport timing, memory and loader phases for the last CLX command.\n"
  "Heap and largest free block refer to Clarice's partition. No machine-wide RAM, CPU usage or Unix process statistics.\n"
  "Status: 0 success, 2 invalid arguments.\n"},
 {"clear",
  "Usage:\n"
  "  clear\n"
  "Clear the terminal display.\n"},
 {"selftest",
  "Usage:\n"
  "  selftest\n"
  "Run built-in ABI/parser/scan checks.\n"},
 {"checks",
  "Usage:\n"
  "  checks\n"
  "Run the application test set.\n"},
 {"scrollbench",
  "Usage:\n"
  "  scrollbench [quick|native|cached]\n"
  "Run rendering diagnostics. No operand runs the default benchmark. quick takes a shorter sample; native and cached compare renderer paths.\n"
  "Additional developer modes require a diagnostic build.\n"},
 {"help",
  "Usage:\n"
  "  help [builtin]\n"
  "  help --help\n"
  "Show shell builtin documentation. Use NAME --help for external tools.\n"
  "Status: 0 success, 1 unknown builtin, 2 too many arguments.\n"},
 {"remote",
  "Usage:\n"
  "  remote status\n"
  "  remote off\n"
  "Report or stop optional Telnet and serial services. See telnetd --help or seriald --help if installed.\n"
  "The services start disabled. One remote session and one executing command at a time; local input remains editable.\n"
  "Status: 0 success, 1 stop deferred/failed, 2 usage.\n"},
 };
 for(unsigned i=0;i<sizeof entries/sizeof entries[0];i++)
  if(!strcmp(topic,entries[i].name))return help_output(1,entries[i].text);
 errsay("help: unknown builtin; try NAME --help for an external utility\n");
 return 1;
}

static int help_command(int argc,char **av){
 if(argc>2)return 2;
 if(argc==2)return builtin_help(!strcmp(av[1],"--help")?"help":av[1]);
 return help_output(1,
  "Clarice: native Macintosh command shell\n"
  "Use help NAME for a builtin and NAME --help for an external tool.\n"
  "Builtins:\n"
  "  cd pwd echo printf export unset set shift read sh . test [ true false : break continue return exit env time command history prefs status clear appinstall remote clarice selftest checks scrollbench help\n"
  "Standard commands:\n"
  "  "
#define CLARICE_COMMAND(name,image) name " "
#include "standard-commands.inc"
#undef CLARICE_COMMAND
  "\n"
  "Up/Down: command history. Tab: completion. Ctrl+C or Command-period: interrupt.\n"
  "Use the scrollbar or pipe help to more to read longer pages.\n"
  "Use hfsattrib to inspect or change HFS metadata.\n");
}
