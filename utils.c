#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <limits.h> // For PATH_MAX
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h> // For time measurement
#include "utils.h"
#include "commands.h"

#define MAX_BG_PROCESSES 1024

pid_t bg_pids[MAX_BG_PROCESSES];
char *bg_commands[MAX_BG_PROCESSES];
int bg_count = 0; // Keep track of number of background processes

char *shell_home_directory; // Global variable to store the shell home directory

void initialize_shell_home_directory() {
    shell_home_directory = getcwd(NULL, 0); // Retrieve current working directory
    if (!shell_home_directory) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

// Get current username
char* get_username() {
    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_name : "unknown"; // Handle case where username can't be retrieved
}

// Get system name
char* get_system_name() {
    static struct utsname unameData;
    uname(&unameData);
    return unameData.nodename;
}

// Check if the current directory is the home directory
int is_home_directory(const char *cwd) {
    return strcmp(cwd, shell_home_directory) == 0;
}

// Execute a command
void execute_command(char *cmd) {
    char *args[4096];
    int argc = 0;

    // Tokenize the command string
    while (*cmd != '\0') {
        while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n') {
            cmd++;
        }
        if (*cmd == '\0') break;

        // Handle quoted arguments
        if (*cmd == '"') {
            cmd++;
            args[argc] = cmd;
            while (*cmd != '"' && *cmd != '\0') cmd++;
            if (*cmd == '\0') {
                printf("Error: Missing closing quote\n");
                return;
            }
            *cmd = '\0';
            cmd++;
        } else {
            args[argc] = cmd;
            while (*cmd != ' ' && *cmd != '\t' && *cmd != '\n' && *cmd != '\0') cmd++;
            if (*cmd != '\0') {
                *cmd = '\0';
                cmd++;
            }
        }
        argc++;
    }

    args[argc] = NULL; // Null-terminate the arguments array

    if (args[0] == NULL) return; // No command entered

    // Log the command
    char log_entry[4096];
    snprintf(log_entry, sizeof(log_entry), "%s", args[0]);
    for (int i = 1; i < argc; i++) {
        snprintf(log_entry + strlen(log_entry), sizeof(log_entry) - strlen(log_entry), " %s", args[i]);
    }

    // Handle built-in commands
    if (strcmp(args[0], "hop") == 0) {
        hop(args, argc);
        return;
    } else if (strcmp(args[0], "reveal") == 0) {
        reveal(args, argc);
        return;
    } else if (strcmp(args[0], "log") == 0) {
        log_command(args, argc);
        return;
    } else if (strcmp(args[0], "proclore") == 0) {
        proclore(args, argc);
        return;
    } 

    // Execute the command using fork and exec
    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) {
            perror("execvp");  // Execution failed
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Parent process
        // Check if it is a background process
        if (args[argc - 1] && strcmp(args[argc - 1], "&") == 0) {
            // Remove '&' from args
            args[--argc] = NULL;
            bg_pids[bg_count] = pid; // Store PID of background process
            bg_commands[bg_count] = strdup(log_entry); // Store command
            if (!bg_commands[bg_count]) {
                perror("strdup");
                exit(EXIT_FAILURE);
            }
            bg_count++;
            printf("Background PID: %d\n", pid);
        } else {
            // Foreground process: measure execution time
            struct timeval start, end;
            gettimeofday(&start, NULL);  // Start time

            int status;
            waitpid(pid, &status, 0);  // Wait for child to finish

            gettimeofday(&end, NULL);  // End time
            long seconds = end.tv_sec - start.tv_sec;
            long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);

            if (micros > 2000000) {  // If the process took more than 2 seconds
                printf("Process %s : %ld seconds\n", args[0], seconds);
            }
        }
    } else {
        perror("fork");
    }
}

// Process commands separated by ';' and handle background execution with '&'
void process_command(char *input) {
    char *command;
    char *rest = input;

    // Tokenize commands based on ';'
    while ((command = strtok_r(rest, ";", &rest))) {
        int background = 0;

        // Check for background execution
        if (strchr(command, '&')) {
            background = 1; // Set background flag
            strtok(command, "&"); // Remove '&' from command
        }

        // Tokenize the command
        char *args[4096];
        int argc = 0;
        char *cmd_copy = strdup(command); // Duplicate command for safe tokenization
        if (!cmd_copy) {
            perror("strdup");
            continue; // Skip this command if allocation fails
        }

        char *token = strtok(cmd_copy, " \t\n"); // Tokenize by whitespace
        while (token != NULL) {
            args[argc++] = token;
            token = strtok(NULL, " \t\n"); // Get next token
        }

        args[argc] = NULL; // Null terminate the arguments array

        // Handle command execution
        if (argc > 0) {
            if (background) {
                pid_t pid = fork();
                if (pid == 0) { // Child process
                    execute_command(command);
                    exit(0);
                } else if (pid > 0) { // Parent process
                    printf("Background PID: %d\n", pid);
                } else {
                    perror("fork");
                }
            } else {
                execute_command(command);
            }
        }

        free(cmd_copy); // Free the duplicated command string
    }
}
