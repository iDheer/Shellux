#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

void activities();
char* get_username();
char* get_system_name();
char *trim_whitespace(char *str);
void process_command(char *input);
void reveal(char **args, int argc);
void add_to_log(const char *command);
void update_foreground_pid(pid_t pid);
void initialize_shell_home_directory(); 
int is_home_directory(const char *cwd);
void log_command(char **args, int argc);
void tokenize_command(char *command, char *args[]);
void execute_command(char *cmd, int is_background);
void execute_piped_commands(char *piped_commands[], int count, int is_background);


#endif