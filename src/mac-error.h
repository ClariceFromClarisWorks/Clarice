/* Common native failures, from Inside Macintosh: Files, File Manager result codes.
 * https://dev.os9.ca/techpubs/mac/Files/Files-301A.html
 * One shell-owned description table; external commands use the existing error API.
 * Unknown codes deliberately have no inferred meaning. */
static const char *mac_error_description(int code){
 switch(code){
 case 0:return "No error";
 case -33:return "Directory is full";
 case -34:return "Volume has no free allocation blocks";
 case -35:return "Volume not found";
 case -36:return "Input/output failure";
 case -37:return "Invalid file or volume name";
 case -38:return "File is not open";
 case -39:return "End of file";
 case -40:return "Position precedes start of file";
 case -42:return "Too many open files";
 case -43:return "File or path not found";
 case -44:return "Volume is hardware write-protected";
 case -45:return "File is locked";
 case -46:return "Volume is software locked";
 case -47:return "File or directory is busy or not empty";
 case -48:return "Name already exists";
 case -49:return "File already open for writing";
 case -50:return "Invalid parameter";
 case -51:return "Invalid file or working-directory reference";
 case -53:return "Volume is offline";
 case -54:return "Cannot open locked file for writing";
 case -59:return "Rename failed";
 case -60:return "Invalid master directory block";
 case -61:return "File access mode does not permit writing";
 case -108:return "Insufficient memory";
 case -120:return "Directory not found";
 case -121:return "Too many open working directories";
 case -122:return "Cannot move a directory inside itself";
 case -123:return "Volume is not HFS";
 case -124:return "Server volume disconnected";
 case -127:return "Internal filesystem error";
 default:return 0;
 }
}
