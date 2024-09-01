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
#include <ctype.h>

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

void execute_command(char *cmd, int is_background) {
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
    } else if (strcmp(args[0], "seek") == 0) {
        seek(args, argc);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) {
            perror("execvp");  // Execution failed
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Parent process
        if (is_background) {
            // Background process
            bg_pids[bg_count] = pid; // Store PID of background process
            bg_commands[bg_count] = strdup(log_entry); // Store command
            if (!bg_commands[bg_count]) {
                perror("strdup");
                exit(EXIT_FAILURE);
            }
            bg_count++;
            printf("Background PID: %d\n", pid);
        } else {
            // Foreground process: measuer execution time
            struct timeval start, end;
            gettimeofday(&start, NULL);  // Start time

            int status;
            waitpid(pid, &status, 0);  // Wait for child to finish

            gettimeofday(&end, NULL);  // End time
            long seconds = end.tv_sec - start.tv_sec;

            if (seconds > 2) { 
                printf("<%s@%s:~ %s : %ld seconds>\n", get_username(), get_system_name(), log_entry, seconds);
            }
        }
    } else {
        perror("fork");
    }
}

// Function to trim whitespace from the beginning and end of a string
char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0'; // Null-terminate after the last non-space character

    return str;
}

void process_command(char *input) {
    char *command;
    char *rest = input;

    // Tokenize commands based on ';'
    while ((command = strtok_r(rest, ";", &rest))) {
        char *sub_command;
        char *rest_sub = command;

        // Count the number of '&' in the command
        int ampersand_count = 0;
        for (char *c = command; *c != '\0'; c++) {
            if (*c == '&') ampersand_count++;
        }

        // Tokenize the command based on '&'
        while ((sub_command = strtok_r(rest_sub, "&", &rest_sub))) {
            sub_command = trim_whitespace(sub_command);
            if (*sub_command == '\0') continue; // Skip empty commands

            // Determine if the command should be executed in the background
            int is_background = (ampersand_count > 0);
            execute_command(sub_command, is_background);

            if (is_background) {
                ampersand_count--; // Decrement the count of background tasks
            }
        }
    }
}
