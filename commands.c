#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "commands.h"

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

// Function to process commands
void process_command(char *input) {
    char *command;
    char *rest = input;

    while ((command = strtok_r(rest, ";", &rest))) {
        char *cmd = strtok(command, "&");
        int background = (strchr(command, '&') != NULL);

        // Handle random spaces and tabs
        char *cleaned_cmd = strtok(cmd, " \t");

        if (strncmp(cleaned_cmd, "cd", 2) == 0) {
            char *path = cleaned_cmd + 3;  // Get the path after 'cd '
            path[strcspn(path, "\n")] = 0;  // Remove newline character
            if (chdir(path) != 0) {
                perror("cd");
            }
        } else if (strncmp(cleaned_cmd, "ls", 2) == 0) {
            list_directory(".");  // List current directory
        } else {
            pid_t pid;
            if (background) {
                pid = fork();
                if (pid == 0) {
                    // Child process
                    execlp(cleaned_cmd, cleaned_cmd, (char *)NULL);
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    // Parent process
                    printf("[%d] %d\n", getpid(), pid);  // Print process ID
                } else {
                    perror("fork");
                }
            } else {
                pid = fork();
                if (pid == 0) {
                    // Child process
                    execlp(cleaned_cmd, cleaned_cmd, (char *)NULL);
                    perror("ERROR");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) {
                    // Parent process waits for the child
                    wait(NULL);
                } else {
                    perror("fork");
                }
            }
        }
    }
}
