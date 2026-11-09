#ifndef SYS6_H
#define SYS6_H
#include <stddef.h>
#define LINE_CAP 1024
#define MAX_ARGS 32
#define PATH_CAP 512
void term_write(const char *s, long n);
void term_puts(const char *s);
void term_present(void);
long app_terminal_read(void *,unsigned long);
void app_page_begin(void);
int app_page(void);
int app_yield(void);
int app_serial_input(int enabled);
int app_serial_key(void);
void app_clear(void);
void app_quit(void);
void app_metrics(int human);
int app_columns(void);
void app_output_batch(int bulk);
void app_profile(unsigned long *,unsigned long *);
/* out must provide at least 32 bytes. */
void format_size(char *out,unsigned long long bytes,int human);
int app_scroll_bench(int quick);
int app_preferences(int argc,char **argv);
void app_run_checks(void);
void shell_init(void);
void shell_profile(void);
void shell_prompt(char *out,size_t cap);
int shell_run(const char *s);
int shell_complete(char *s, int *cursor);
int history_get(int back, char *out);
int history_search(const char *needle, int start, char *out);
void history_shutdown(void);
extern char shell_cwd[PATH_CAP];
extern int shell_status;
extern unsigned long shell_commands;
unsigned long asm_length(const char *s);
int asm_contains(const char *s, const char *needle);
void asm_glyph(unsigned char *dest,const unsigned char *src,unsigned long stride,unsigned long height);
#endif

/* Interactive screen owners suppress the passive busy overlay. */
void app_activity_interactive(int);

int shell_state_path(char *out,const char *file,const char *legacy);

int shell_rename(const unsigned char *source,const unsigned char *newname);
void shell_temp_path(char *out,const char *file);

int cli_pause_ticks(unsigned long ticks);

int app_pager_begin(void),app_pager_end(int);
