#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <limits.h> // For PATH_MAX
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"
#include "commands.h"

char *shell_home_directory; // yeh apna global variable to store the shell home directory

void initialize_shell_home_directory() {
    shell_home_directory = getcwd(NULL, 0); // current working directory ka retrieval
}

// current username
char* get_username() {
    struct passwd *pw = getpwuid(getuid());
    return pw->pw_name;
}

// system name
char* get_system_name() {
    static struct utsname unameData;
    uname(&unameData);
    return unameData.nodename;
}

// if the current directory is the home directory
int is_home_directory(const char *cwd) {
    return strcmp(cwd, shell_home_directory) == 0;
}

// Now how to execute a command
void execute_command(char *cmd) {
    char *args[4096];
    int argc = 0;

    while (*cmd != '\0') { // 
        // Skip leading whitespaces
        while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n') { // leading whitespaces ko skip karna
            cmd++; // we are iterating over the command string
        }

        if (*cmd == '\0') {
            break;
        }

        // Handle quoted arguments 
        if (*cmd == '"') {
            cmd++;
            args[argc] = cmd;
            while (*cmd != '"' && *cmd != '\0') {
                cmd++;
            }
            if (*cmd == '\0') {
                printf("Error: Missing closing quote\n");
                return;
            }
            *cmd = '\0';
            cmd++;
        } else {
            args[argc] = cmd;
            while (*cmd != ' ' && *cmd != '\t' && *cmd != '\n' && *cmd != '\0') {
                cmd++;
            }
            if (*cmd != '\0') {
                *cmd = '\0';
                cmd++;
            }
        }

        argc++;
    }

    args[argc] = NULL; // Null-terminate the arguments array is important

    if (args[0] == NULL) {
        return; // No command entered
    }
    if (strcmp(args[0], "hop") == 0) {
        hop(args, argc);
        return;
    }
    if (strcmp(args[0], "reveal") == 0) {
        reveal(args, argc);
    }
    if (strcmp(args[0], "log") == 0) {
        log_command(args, argc);
    }

    // Debugging: Print the command and arguments
    printf("Executing command: %s\n", args[0]);
    for (int i = 0; i < argc; i++) {
        printf("Arg[%d]: '%s'\n", i, args[i]);
    }
}

//  this is my function to process commands separated by ';' and handle background execution with '&'
void process_command(char *input) {
    char *command;
    char *rest = input;

    // pehle tokenize kara ';' ke basis pe
    while ((command = strtok_r(rest, ";", &rest))) {
        int background = 0;

        // Check if the command has '&' for background execution
        if (strchr(command, '&')) { // agr '&' hai toh background execution
            background = 1; // background flag set kardiya
            strtok(command, "&"); // '&' ko remove kardiya command se
        }

        // Tokenize the command
        char *args[4096];
        int argc = 0;
        char *cmd_copy = strdup(command); // Duplicate command for safe tokenization
        char *token = strtok(cmd_copy, " \t\n"); // ab humne white spaces, tabs, newlines ke basis pe tokenize kara

        while (token != NULL) { // this is the loop to tokenize the command such that we get the arguments in the args array
            args[argc++] = token;
            token = strtok(NULL, " \t\n");
        }

        args[argc] = NULL; // null terminate the arguments array karna zaroori hai

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

        free(cmd_copy);
    }
}
