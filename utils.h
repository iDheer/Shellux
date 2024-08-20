#ifndef UTILS_H
#define UTILS_H

extern char *shell_home_directory; // Global variable for the shell home directory

char* get_username();
char* get_system_name();
void execute_command(char *cmd); 
void process_command(char *input);
void initialize_shell_home_directory(); 
int is_home_directory(const char *cwd);
void reveal(char **args, int argc);
void log_command(char **args, int argc);
void add_to_log(const char *command);


#endif
