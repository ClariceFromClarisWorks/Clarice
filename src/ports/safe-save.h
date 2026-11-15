#ifndef CLARICE_SAFE_SAVE_H
#define CLARICE_SAFE_SAVE_H
/* Opt-in single-invocation HFS replacement. Returned token belongs to host.
   begin returns a native: staging path, requiring a buffer of at least 264 bytes.
   Caller writes/closes the stage with existing stdio, then commits only on success.
   commit consumes token on success OR failure; abort consumes token, preserves errno.
   Do not access stage after either. Host aborts abandoned tokens on invocation exit.
   Errors use errno; recovery paths are reported if cleanup/rollback fails.
   No concurrency, atomic replacement, or power-loss guarantees. */
void *cli_replace_begin(const char *,char *,unsigned long);
int cli_replace_commit(void *);
void cli_replace_abort(void *);
#endif
