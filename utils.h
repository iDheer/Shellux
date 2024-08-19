#ifndef UTILS_H
#define UTILS_H

extern char *shell_home_directory; // To store the home directory for the shell

char* get_username();
char* get_system_name();
int is_home_directory(const char *cwd);
void initialize_shell_home_directory(); 

#endif
