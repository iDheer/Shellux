#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "commands.h"

// Global variables for managing foreground processes
pid_t fg_pid = 0; // Foreground process ID
int bg_count = 0; // Background process counter

void handle_sigchld(int sig) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("\nBackground process [%d] exited with status %d\n", pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("\nBackground process [%d] killed by signal %d\n", pid, WTERMSIG(status));
        }
        bg_count--;
    }
}

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGINT); // Send SIGINT to foreground process
    }
}

// Signal handler for SIGTSTP (Ctrl+Z)
void handle_sigtstp(int sig) {
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP); // Send SIGTSTP to foreground process
    }
}

// Function to list directory contents with color coding
void list_directory(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        struct stat file_stat;
        char full_path[1024];

        // Construct the full path
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        stat(full_path, &file_stat);

        if (S_ISDIR(file_stat.st_mode)) {
            // Directory - Blue
            printf("\033[0;34m%s\033[0m\n", entry->d_name);
        } else if (file_stat.st_mode & S_IXUSR) {
            // Executable - Green
            printf("\033[0;32m%s\033[0m\n", entry->d_name);
        } else {
            // Regular file - White
            printf("\033[0;37m%s\033[0m\n", entry->d_name);
        }
    }

    closedir(dir);
}

void execute_command(char *cmd) {
    printf("cmd value from execute_command: '%s'\n", cmd);

    char *args[100];
    int argc = 0;
    char *redirect_file = NULL;
    int redirect_type = 0; // 1 for '>', 2 for '>>', 3 for '<'

    // Tokenize command and arguments
    while ((args[argc] = strtok(cmd, " \t\n")) != NULL) {
        cmd = NULL;
        if (strcmp(args[argc], ">") == 0) {
            redirect_type = 1;
            redirect_file = strtok(NULL, " \t\n");
            break;
        } else if (strcmp(args[argc], ">>") == 0) {
            redirect_type = 2;
            redirect_file = strtok(NULL, " \t\n");
            break;
        } else if (strcmp(args[argc], "<") == 0) {
            redirect_type = 3;
            redirect_file = strtok(NULL, " \t\n");
            break;
        }
        argc++;
    }
    args[argc] = NULL; // Null-terminate the arguments array

    if (args[0] == NULL) {
        return; // No command entered
    }

    // Handle custom command: `ls`
    if (strcmp(args[0], "ls") == 0) {
        char *path = argc > 1 ? args[1] : ".";
        list_directory(path);
        return;
    }

    // Handle redirection if necessary
    if (redirect_file) {
        int fd;
        if (redirect_type == 1) {
            fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else if (redirect_type == 2) {
            fd = open(redirect_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else if (redirect_type == 3) {
            fd = open(redirect_file, O_RDONLY);
        }
        if (fd == -1) {
            perror("open");
            return;
        }
        if (redirect_type == 3) {
            dup2(fd, STDIN_FILENO);  // Redirect input
        } else {
            dup2(fd, STDOUT_FILENO); // Redirect output
        }
        close(fd);
    }

    // For other commands, fallback to execvp
    execvp(args[0], args);
    perror("execvp"); // If execvp fails, print error message
    exit(EXIT_FAILURE);
}

void process_command(char *input) {
    char *command;
    char *rest = input;

    // Loop through commands separated by ';'
    while ((command = strtok_r(rest, ";", &rest))) {
        int background = 0;

        // Check if the command has '&' for background execution
        if (strchr(command, '&')) {
            background = 1;
            strtok(command, "&");
        }

        // Tokenize the command
        char *args[100];
        int argc = 0;
        char *cmd_copy = strdup(command); // Duplicate command for safe tokenization
        char *token = strtok(cmd_copy, " \t\n");

        while (token != NULL) {
            args[argc++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[argc] = NULL;

        // Handle 'cd' command
        if (args[0] && strcmp(args[0], "cd") == 0) {
            if (argc < 2) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd failed");
                }
            }
            free(cmd_copy); // Free the duplicated command
            continue; // Skip the rest of the loop to avoid forking
        }

        // Handle piping
        if (strchr(command, '|')) {
            char *pipe_parts[2];
            int pipe_count = 0;
            while ((pipe_parts[pipe_count] = strsep(&command, "|")) != NULL && pipe_count < 2) {
                pipe_count++;
            }

            if (pipe_count == 2) {
                int pipefd[2];
                pipe(pipefd);

                pid_t pid1 = fork();
                if (pid1 == 0) {
                    close(pipefd[0]);
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                    execute_command(pipe_parts[0]);
                    exit(EXIT_SUCCESS);
                }

                pid_t pid2 = fork();
                if (pid2 == 0) {
                    close(pipefd[1]);
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    execute_command(pipe_parts[1]);
                    exit(EXIT_SUCCESS);
                }

                close(pipefd[0]);
                close(pipefd[1]);
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
            }
            free(cmd_copy); // Free the duplicated command
            continue;
        }

        // Fork and execute the command
        pid_t pid = fork();
        if (pid == 0) {
            if (background) {
                printf("[%d] %d\n", getpid(), pid);
                bg_count++;
            }
            execute_command(args[0]);
            exit(EXIT_SUCCESS); // Ensure child exits after executing command
        } else if (pid > 0) {
            fg_pid = pid;
            if (!background) {
                waitpid(pid, NULL, 0);
            }
            fg_pid = 0;
        } else {
            perror("fork");
        }

        free(cmd_copy); // Free the duplicated command
    }
}
