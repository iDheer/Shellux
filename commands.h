#ifndef COMMANDS_H
#define COMMANDS_H

#include "globals.h"

char kbhit();
void load_log(void);
void save_log(void);
void init_log(void);
void iMan(char *cmd);
void clear_log(void);
void display_log(void);
void cleanup_log(void);
void print_latest_pid();
void handle_exit(int sig);
void err(const char *s);
void disableRawMode();
void enableRawMode();
void neoexec(int time);
void neonate(char *cmd);
void fg_process(pid_t pid);
void bg_process(pid_t pid);
void hop(char **args, int argc);
void seek(char **args, int argc);
void execute_from_log(int index);
void reveal(char **args, int argc);
void proclore(char **args, int argc);
void add_to_log(const char *command);
void print_process_details(pid_t pid);
void log_command(char **args, int argc);
int compare(const void *a, const void *b);
void ping_process(pid_t pid, int signal_number);
void set_raw_mode(struct termios *orig_termios);
void reset_terminal(struct termios *orig_termios);
void print_file_details(const char *path, const char *filename);
void search_directory(const char *base_dir, const char *search_term, char results[MAX_RESULTS][MAX_PATH], int *result_count, int d_flag, int f_flag, int e_flag, char *found_file, char *found_dir, int *file_count, int *dir_count);

#endif 