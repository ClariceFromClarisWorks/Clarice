/* Invocation-local scratch, released with the CLX image. */
#define COMMON_BUFSIZE 1024
extern union bb_scratch {long align; char bytes[COMMON_BUFSIZE];} bb_scratch;
#define bb_common_bufsiz1 bb_scratch.bytes
#define setup_common_bufsiz() ((void)0)
#define BUILD_BUG_ON(c) ((void)sizeof(char[1-2*!!(c)]))
