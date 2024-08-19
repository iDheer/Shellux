#ifndef COMMANDS_H
#define COMMANDS_H

#include <signal.h>

// Global variables
extern pid_t fg_pid; // Foreground process ID
extern int bg_count; // Background process counter

// Function declarations
void list_directory(const char *path);
void process_command(char *input);
void handle_sigchld(int sig); // Signal handler for background process termination
void handle_sigint(int sig);  // Signal handler for Ctrl+C
void handle_sigtstp(int sig); // Signal handler for Ctrl+Z
void execute_command(char *cmd); // Function to execute a single command (handling redirection)

#endif
