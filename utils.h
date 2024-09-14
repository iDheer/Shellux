#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

void activities();
char* get_username();
char* get_system_name();
void process_command(char *input);
void execute_command(char *cmd, int is_background);
void tokenize_command(char *command, char **args, int *is_background);
void execute_piped_commands(char *piped_commands[], int count, int is_background);

#endif