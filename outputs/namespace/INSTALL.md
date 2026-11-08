# Install Clarice

Use Clarice.dsk (an 800 KB HFS application disk) or the Clarice.bin MacBinary file. Preserve both forks when transferring the application to a classic Macintosh. Copy Clarice to a convenient application folder and launch it from Finder.

Clarice includes its own first-run data installer. It locates the System Folder of the running system using the classic OS working-directory information, then creates **Clarice Data** there. No extensions, startup applications, OS cache settings or network configuration are installed or changed. The application itself may remain outside the System Folder.

The installed root contains bin, etc, tmp, dev, mnt and usr/bin. The initial etc/profile sets PATH=/bin:/usr/bin and includes a commented example for HOSTNAME. Subsequent launches preserve existing folders and profile content. A writable HFS system volume is required for first-run setup. The current root can be inspected directly in Finder; links are Clarice-specific files, not Finder aliases.

Inside Clarice, / refers to Clarice Data. /mnt is a virtual list of currently mounted Macintosh volumes; it does not copy their contents into Clarice Data. /dev is reserved for future endpoints, not an implemented device driver interface.

Edit /etc/profile with a compatible text editor, or write it with shell redirection. It is read once at startup; `. /etc/profile` applies it immediately. A sample:

```sh
export PATH=/bin:/usr/bin
export HOSTNAME=macplus
```

The prompt then shows `macplus directory >`. Without HOSTNAME it shows `directory >`. `pwd` prints the full path. Networking-based hostname discovery is not implemented in this build.

This development build uses a 384 KiB application partition and targets the Macintosh Plus with 4 MB and MultiFinder. The tested guest is System 6.0.8, confirmed by Finder and installation media despite SysEnvirons reporting 0607. See outputs/system608/TEST-REPORT.md and outputs/ls-opt2/TEST-REPORT.md. No System software, ROM or emulator is included.

Earlier development folders named Sys6Unix or Terminal Data are not automatically moved or deleted. Existing personal data is not migrated or replaced by this installer. New data uses Clarice Data exclusively.
