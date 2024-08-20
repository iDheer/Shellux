#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <linux/limits.h> // For PATH_MAX
#include "commands.h"
#include "utils.h"
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

static char prev_dir[PATH_MAX] = ""; // Global variable for previous directory
static char *command_log[15]; // Array to store the command log, document mein likha hai max 15 commands can be stored
static int log_count = 0;

void hop(char **args, int argc) {
    char *target_dir;
    char cwd[PATH_MAX];

    // printf("argc: %d\n", argc); hop khud ek arguement consider karega, so argc 1 se start hoga

    // for(int i=0; i<argc; i++){
    //     printf("%s\n", args[i]);
    // }

    // now i need to implement for absolute paths, he one thing i know is that it will always have argc =2 as 1 is hop command and other is an entire path without spaces and the it will have / as a character at the start
    // 1 way in which i could be given the arguements is like hop /home/ineshdheer/Downloads, this case has already been handled and the other way for absolute paths is like hop ~/project, this is what i need to fix 

    if (args[1][0] == '~' && args[1][1] == '/') {
            // Handle case where path starts with ~/

        char *target_dir = args[1] + 2; // Skip '~/' to get the relative path from home
        char *username = get_username();

        // Calculate the size needed for the final directory string
        size_t final_dir_size = strlen("/home/") + strlen(username) + strlen("/") + strlen(target_dir) + 1;

        // Allocate memory for the final directory path
        char *final_dir = malloc(final_dir_size);
        if (final_dir == NULL) {
            perror("malloc failed");
            return;
        }

        // Construct the final directory path
        snprintf(final_dir, final_dir_size, "/home/%s/%s", username, target_dir);
        printf("%s\n", final_dir);

        // Attempt to change to the final directory
        if (chdir(final_dir) == -1) {
            perror("chdir");
            free(final_dir);
            return;
        }

        free(final_dir); // Free the allocated memory after use
        return;

    }

    for (int i = 1; i < argc; ++i) {
        target_dir = args[i];

        // Handle '.' for the current directory (no change needed)
        if (strcmp(target_dir, ".") == 0) {
            target_dir = cwd;
        }
        // Handle '..' for the parent directory
        else if (strcmp(target_dir, "..") == 0) {
            if (chdir("..") == -1) {
                perror("chdir");
                return;
            }
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                perror("getcwd");
                return;
            }
            printf("%s\n", cwd);
            continue;
        }
        // Handle '~' for the shell's home directory
        else if (strcmp(target_dir, "~") == 0) {
            target_dir = shell_home_directory;
        }
        // Handle '-' for the previous directory
        else if (strcmp(target_dir, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                printf("No previous directory\n");
                return;
            }   
            target_dir = prev_dir;
        }
        
        // Save the current directory before changing it
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return;
        }

        // Change the directory
        if (chdir(target_dir) == -1) {
            perror("chdir");
            return;
        }

        // Update the previous directory
        strncpy(prev_dir, cwd, sizeof(prev_dir) - 1);
        prev_dir[sizeof(prev_dir) - 1] = '\0'; // Ensure null-termination

        // Print the new working directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return;
        }
        printf("%s\n", cwd);
    }

    // If no arguments are provided, hop to the home directory
    if (argc == 1) {
        target_dir = shell_home_directory;
        if (chdir(target_dir) == -1) {
            perror("chdir");
            return;
        }

        // Print the new working directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return;
        }
        printf("%s\n", cwd);
    }
}

    // Add a command to the log
    void add_to_log(const char *command) {
        if (log_count > 0 && strcmp(command_log[log_count - 1], command) == 0) {
            return; // Don't add if it's the same as the last command
        }

        if (log_count < 15) {
            command_log[log_count++] = strdup(command);
        } else {
            free(command_log[0]);
            memmove(&command_log[0], &command_log[1], sizeof(char*) * (14)); // 15 - 1
            command_log[14] = strdup(command); // 14 here is 15 - 1 
        }
    }

    // Display the log
    void display_log() {
        for (int i = 0; i < log_count; ++i) {
            printf("%d: %s\n", log_count - i, command_log[i]);
        }
    }

    // Clear the log
    void clear_log() {
        for (int i = 0; i < log_count; ++i) {
            free(command_log[i]);
        }
        log_count = 0;
    }

    // Execute a command from the log
    void execute_from_log(int index) {
        if (index < 1 || index > log_count) {
            printf("Error: Invalid log index\n");
            return;
        }

        char *command_to_execute = command_log[log_count - index];
        printf("Executing: %s\n", command_to_execute);
        process_command(command_to_execute);
    }

    void log_command(char **args, int argc) {
        if (argc == 1) {
            display_log();
        } else if (argc == 2 && strcmp(args[1], "purge") == 0) {
            clear_log();
        } else if (argc == 3 && strcmp(args[1], "execute") == 0) {
            int index = atoi(args[2]);
            execute_from_log(index);
        } else {
            printf("Error: Invalid log command\n");
        }
    }

    // Helper function to print file details for -l flag
    void print_file_details(const char *path, const char *filename) {
        struct stat fileStat;
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

        if (stat(fullpath, &fileStat) == -1) {
            perror("stat");
            return;
        }

        // Print file type and permissions
        printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
        printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
        printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
        printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
        printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
        printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
        printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
        printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
        printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
        printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

        // Print number of links
        printf(" %lu", fileStat.st_nlink);

        // Print user and group name
        struct passwd *pwd = getpwuid(fileStat.st_uid);
        struct group *grp = getgrgid(fileStat.st_gid);
        printf(" %s %s", pwd->pw_name, grp->gr_name);

        // Print file size
        printf(" %5ld", fileStat.st_size);

        // Print last modification time
        char timebuff[80];
        strftime(timebuff, sizeof(timebuff), "%b %d %H:%M", localtime(&fileStat.st_mtime));
        printf(" %s", timebuff);

        // Print file name with color coding
        if (S_ISDIR(fileStat.st_mode)) {
            printf(" \033[1;34m%s\033[0m\n", filename); // Blue for directories
        } else if (fileStat.st_mode & S_IXUSR) {
            printf(" \033[1;32m%s\033[0m\n", filename); // Green for executables
        } else {
            printf(" \033[0;37m%s\033[0m\n", filename); // White for regular files
        }
    }

    // Implementation of the 'reveal' command
    void reveal(char **args, int argc) {
        int show_all = 0;
        int show_long = 0;
        char *target_dir = ".";
        int optind = 1;

        // Parse flags
        while (optind < argc && args[optind][0] == '-') {
            for (int j = 1; args[optind][j] != '\0'; ++j) {
                if (args[optind][j] == 'a') {
                    show_all = 1;
                } else if (args[optind][j] == 'l') {
                    show_long = 1;
                } else {
                    printf("Error: Invalid flag '%c'\n", args[optind][j]);
                    return;
                }
            }
            optind++;
        }

        if (optind < argc) {
            target_dir = args[optind];
        }

        DIR *dir = opendir(target_dir);
        if (dir == NULL) {
            perror("opendir");
            return;
        }

        struct dirent *entry;
        char *entries[1024];
        int count = 0;

        // Read and store directory entries
        while ((entry = readdir(dir)) != NULL) {
            if (!show_all && entry->d_name[0] == '.') {
                continue;
            }
            entries[count++] = strdup(entry->d_name);
        }
        closedir(dir);

        // Sort entries lexicographically
        qsort(entries, count, sizeof(char*), (int (*)(const void*, const void*)) strcmp);

        // Print entries
        for (int i = 0; i < count; ++i) {
            if (show_long) {
                print_file_details(target_dir, entries[i]);
            } else {
                printf("%s\n", entries[i]);
            }
            free(entries[i]);
        }
    }
