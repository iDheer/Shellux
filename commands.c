#include "utils.h"
#include "commands.h"
#include "prompt.h"

static char prev_dir[PATH_MAX] = ""; // Global variable for previous directory
char *command_log[MAX_LOG_SIZE];
int log_count = 0;

    void hop(char **args, int argc) {
        char *target_dir;
        char cwd[PATH_MAX];

        // now i need to implement for absolute paths, he one thing i know is that it will always have argc =2 as 1 is hop command and other is an entire path without spaces and the it will have / as a character at the start
        // 1 way in which i could be given the arguements is like hop /home/ineshdheer/Downloads, this case has already been handled and the other way for absolute paths is like hop ~/project, this is what i need to fix 

        if (argc == 1 && strcmp(args[0], "hop") == 0) {
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
            return;
        }

        if (args[1][0] == '~' && args[1][1] == '/') {
                // Handle case where path starts with ~/

            char *target_dir = args[1] + 2; // Skip '~/' to get the relative path from home
            char *username = get_username();

            // Calculate the size needed for the final directory string
            size_t final_dir_size = strlen("/home/") + strlen(username) + strlen("/") + strlen(target_dir) + 1;

            // Allocate memory for the final directory path
            char *final_dir = malloc(final_dir_size);
            if (final_dir == NULL) {
                free(final_dir);
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

            free(final_dir); 
            return;

        }

        // printf("%s", args[0]);

        for (int i = 1; i < argc; ++i) {
            target_dir = args[i];

            // Handle '.' for the current directory 
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
    }

    void load_log() {
        FILE *log_file = fopen(LOG_FILE_PATH, "r");
        if (!log_file) return;

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), log_file)) {
            buffer[strcspn(buffer, "\n")] = 0; // Remove newline character
            command_log[log_count++] = strdup(buffer);
            if (log_count >= MAX_LOG_SIZE) break;
        }
        fclose(log_file);
    }

    void save_log() {
        FILE *log_file = fopen(LOG_FILE_PATH, "w");
        if (!log_file) {
            perror("Failed to open log file");
            return;
        }

        for (int i = 0; i < log_count; ++i) {
            fprintf(log_file, "%s\n", command_log[i]);
            // printf("wrote %s\n", command_log[i]);
            free(command_log[i]);  // Freeing here
            command_log[i] = NULL;  // Ensuring pointer is nullified after freeing
        }

        log_count = 0;  // Reset log count after saving
        fflush(log_file);  // Ensure all data is written to the file
        fclose(log_file);
    }


    // Initialize the log at startup
    void init_log() {
        FILE *log_file = fopen(LOG_FILE_PATH, "a");
        if (log_file) {
            fclose(log_file); // Close immediately after ensuring the file exists
        }
        load_log(); // Load the log from the file
    }

    // Cleanup and save the log at shutdown
    void cleanup_log() {
        save_log(); // Log saving already frees memory, so clear_log() is no longer needed
    }

    // Add a command to the log
    void add_to_log(const char *command) {
        if (log_count > 0 && strcmp(command_log[log_count - 1], command) == 0) {
            return; // Don't add if it's the same as the last command
        }
        if (strstr(command, "log") != NULL) {
            return; // Don't add if the command contains the word 'log' anywhere
        }
        if (strcmp(command, "log purge") == 0) {
            return; // Don't add if the command is 'log purge'
        }

        // If the log is full, free the oldest entry before shifting
        if (log_count >= MAX_LOG_SIZE) {
            free(command_log[0]);
            memmove(&command_log[0], &command_log[1], sizeof(char*) * (MAX_LOG_SIZE - 1));
            log_count--;
        }

        command_log[log_count] = strdup(command);
        if (command_log[log_count] == NULL) {
            perror("strdup");
            return;
        }
        log_count++;
    }

    // Display the log
    void display_log() {
        for (int i = 0; i < log_count; ++i) {
            printf("%d: %s\n", i, command_log[log_count - i - 1]); // Display the log in reverse order with index
        }
    }

    // Clear the log
    void clear_log() {
    for (int i = 0; i < log_count; ++i) {
        free(command_log[i]);
        command_log[i] = NULL;
    }
    log_count = 0;
    save_log(); // Save the cleared log to the file
}

    // Execute a command from the log
    void execute_from_log(int index) {
        if (index < 0 || index >= log_count) { 
            printf("Error: Invalid log index\n");
            return;
        }

        char *command_to_execute = command_log[index];
        printf("Executing: %s\n", command_to_execute);

        process_command(command_to_execute);
        
    }

    // Handle the log command
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

// Function to compare strings for qsort
int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to print file details
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

    // number of links to the file jo ki basically number of hard links to the file hote hain
    printf(" %lu", fileStat.st_nlink);

    //user and group name of the file
    struct passwd *pwd = getpwuid(fileStat.st_uid);
    struct group *grp = getgrgid(fileStat.st_gid);
    printf(" %s %s", pwd ? pwd->pw_name : "???", grp ? grp->gr_name : "???");

    //  file size
    printf(" %5ld", fileStat.st_size);

    // last modification time
    char timebuff[80];
    strftime(timebuff, sizeof(timebuff), "%b %d %H:%M", localtime(&fileStat.st_mtime));
    printf(" %s", timebuff);

    // file name with color codes ke saath 
    if (S_ISDIR(fileStat.st_mode)) {
        printf(" \033[1;34m%s\033[0m\n", filename); // Blue for directories
    } else if (fileStat.st_mode & S_IXUSR) {
        printf(" \033[1;32m%s\033[0m\n", filename); // Green for executables
    } else {
        printf(" \033[0;37m%s\033[0m\n", filename); // White for regular files
    }
}

void reveal(char **args, int argc) {
    int optind = 1;
    int show_all = 0;
    int show_long = 0;
    char *target_dir = ".";

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
        target_dir = args[optind]; // Get target directory
    }

    // Handle special symbols
    if (strcmp(target_dir, ".") == 0) {
        target_dir = getcwd(NULL, 0); // Current working directory
    } else if (strcmp(target_dir, "..") == 0 || strstr(target_dir, "../") != NULL) {
        char *resolved_path = realpath(target_dir, NULL); // Resolve relative paths
        if (resolved_path == NULL) {
            perror("realpath");
            return;
        }
        target_dir = resolved_path;
    } else if (target_dir[0] == '~' && target_dir[1] != '/') {
        target_dir = shell_home_directory; // Shell's home directory
    } else if (target_dir[0] == '~' && target_dir[1] == '/') {
        char *username = get_username();  
        char *offset = target_dir + 2;    
        size_t final_dir_size = strlen("/home/") + strlen(username) + strlen("/") + strlen(offset) + 1;
        char *final_dir = malloc(final_dir_size);
        if (final_dir == NULL) {
            perror("malloc failed");
            return;
        }
        snprintf(final_dir, final_dir_size, "/home/%s/%s", username, offset);
        target_dir = final_dir;
    } else if (strcmp(target_dir, "-") == 0) {
        if (strlen(prev_dir) == 0) {
            printf("Error: No previous directory to reveal.\n");
            return;
        }
        target_dir = prev_dir; // Previous directory handling
    } else if (target_dir[0] == '/') {
        // Do nothing for absolute path
    } else {
        printf("Error: Invalid directory '%s'\n", target_dir);
        return;        
    }

    DIR *dir = opendir(target_dir);
    if (dir == NULL) {
        perror("opendir");
        free(target_dir);
        return;
    }

    struct dirent *entry;
    char **entries = malloc(sizeof(char *) * 1000);
    if (!entries) {
        perror("malloc failed");
        closedir(dir);
        free(target_dir);
        return;
    }

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
    qsort(entries, count, sizeof(char*), compare);

    // Print entries with color coding regardless of flags
    for (int i = 0; i < count; ++i) {
        if (show_long) {
            print_file_details(target_dir, entries[i]);
        } else {
            // Always apply color coding
            struct stat fileStat;
            char fullpath[PATH_MAX];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", target_dir, entries[i]);
            stat(fullpath, &fileStat); // Get file stats for color coding

            // Print file name with color coding
            if (S_ISDIR(fileStat.st_mode)) {
                printf("\033[1;34m%s\033[0m\n", entries[i]); // Blue directories
            } else if (fileStat.st_mode & S_IXUSR) {
                printf("\033[1;32m%s\033[0m\n", entries[i]); // Green executables
            } else {
                printf("\033[0;37m%s\033[0m\n", entries[i]); // White for others
            }
        }
        free(entries[i]);
    }

    // Free dynamically allocated memory for target_dir
    if (target_dir != args[optind]) {
        free(target_dir);
    }

    free(entries);
}

void proclore(char **args, int argc) {
    char proc_path[PATH_MAX];
    char buffer[1024];
    char stat_path[PATH_MAX];
    int pid = getpid();  // Default to the current shell process if no PID is provided

    if (argc == 2) {
        pid = atoi(args[1]);
    }

    // Construct the /proc/[pid] path
    snprintf(proc_path, sizeof(proc_path), "/proc/%d", pid);

    if (snprintf(stat_path, sizeof(stat_path), "%s/stat", proc_path) >= sizeof(stat_path)) {
        fprintf(stderr, "Path too long to append /stat\n");
        return;
    }

    // Open and read the /proc/[pid]/stat file
    FILE *stat_file = fopen(stat_path, "r");
    if (stat_file == NULL) {
        perror("fopen");
        return;
    }

    if (fgets(buffer, sizeof(buffer), stat_file) != NULL) {
        int process_id, pgrp, session, tty_nr, tpgid;
        char comm[256], state;
        unsigned long vsize;

        sscanf(buffer, "%d %s %c %*d %d %d %d %d %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %lu",
               &process_id, comm, &state, &pgrp, &session, &tty_nr, &tpgid, &vsize);

        // Determine if the process is foreground/background
        char *status_str = "Unknown";
        if (session == pgrp && tty_nr != 0) {
            status_str = (tpgid == pid) ? "R+" : "R";
        } else {
            status_str = (state == 'S') ? "S" : "R";
        }

        // Print the process information
        printf("pid : %d\n", process_id);
        printf("process status : %s\n", status_str);
        printf("Process Group : %d\n", pgrp);
        printf("Virtual memory : %lu\n", vsize);

        // Get the executable path
        char exe_path[PATH_MAX];
        if (snprintf(exe_path, sizeof(exe_path), "%s/exe", proc_path) >= sizeof(exe_path)) {
            fprintf(stderr, "Path too long to append /exe\n");
            fclose(stat_file);
            return;
        }

        ssize_t len = readlink(exe_path, buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            printf("executable path : %s\n", buffer);
        } else {
            printf("executable path : unknown\n");
        }
    } else {
        printf("Failed to read process information.\n");
    }
    fclose(stat_file);
}

#define BLUE "\033[1;34m"
#define GREEN "\033[1;32m"
#define RESET "\033[0m"

void search_directory(const char *base_dir, const char *search_term, char results[MAX_RESULTS][MAX_PATH], int *result_count, int d_flag, int f_flag, int e_flag, char *found_file, char *found_dir, int *file_count, int *dir_count) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    char path[MAX_PATH];

    if (!(dir = opendir(base_dir))) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", base_dir, entry->d_name);
        stat(path, &path_stat);

        // Calculate the relative path from the base directory
        const char *relative_path = path + strlen(base_dir) + 1;

        if (S_ISDIR(path_stat.st_mode)) {
            if (strncmp(entry->d_name, search_term, strlen(search_term)) == 0 && (d_flag || (!d_flag && !f_flag))) {
                (*dir_count)++;
                strncpy(found_dir, path, sizeof(path));
                if (*result_count < MAX_RESULTS) {
                    snprintf(results[*result_count], MAX_PATH, BLUE "%s" RESET, relative_path);
                    (*result_count)++;
                }
            }
            // Recursively search in this directory
            search_directory(path, search_term, results, result_count, d_flag, f_flag, e_flag, found_file, found_dir, file_count, dir_count);
        } else if (S_ISREG(path_stat.st_mode)) {
            if (strncmp(entry->d_name, search_term, strlen(search_term)) == 0 && (f_flag || (!d_flag && !f_flag))) {
                (*file_count)++;
                strncpy(found_file, path, sizeof(path));
                if (*result_count < MAX_RESULTS) {
                    snprintf(results[*result_count], MAX_PATH, GREEN "%s" RESET, relative_path);
                    (*result_count)++;
                }
            }
        }
    }

    closedir(dir);
}

void seek(char **args, int argc) {
    int d_flag = 0, f_flag = 0, e_flag = 0;
    char *target_name = NULL;
    char *target_dir = ".";
    int file_count = 0, dir_count = 0;
    char found_file[MAX_PATH] = {0};
    char found_dir[MAX_PATH] = {0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(args[i], "-d") == 0) {
            d_flag = 1;
        } else if (strcmp(args[i], "-f") == 0) {
            f_flag = 1;
        } else if (strcmp(args[i], "-e") == 0) {
            e_flag = 1;
        } else if (!target_name) {
            target_name = args[i];
        } else {
            target_dir = args[i];
        }
    }

    if (d_flag && f_flag) {
        printf("Invalid flags! Cannot use both -d and -f at the same time.\n");
        return;
    }

    if (!target_name) {
        printf("No target name provided!\n");
        return;
    }

    char results[MAX_RESULTS][MAX_PATH];
    int result_count = 0;

    search_directory(target_dir, target_name, results, &result_count, d_flag, f_flag, e_flag, found_file, found_dir, &file_count, &dir_count);

    if (result_count == 0) {
        printf("No match found!\n");
    } else {
        for (int i = 0; i < result_count; i++) {
            printf("%s\n", results[i]);
        }
    }

    if (e_flag) {
        if (file_count == 1 && dir_count == 0) {
            FILE *file = fopen(found_file, "r");
            if (!file) {
                perror("fopen");
                return;
            }

            char ch;
            while ((ch = fgetc(file)) != EOF) {
                putchar(ch);
            }
            fclose(file);
        } else if (dir_count == 1 && file_count == 0) {
            if (chdir(found_dir) == 0) {
                char cwd[MAX_PATH];
                getcwd(cwd, sizeof(cwd));
                printf("Changed directory to: %s\n", cwd);
            } else {
                perror("chdir");
            }
        } else {
            printf("No match found!\n");
        }
    }
}

// Function to send signal to a process
void ping_process(pid_t pid, int signal_number) {
    // Take modulo 32 of the signal number
    int actual_signal = signal_number % 32;

    // Check if the process with given PID exists
    if (kill(pid, 0) == -1) {
        perror("No such process found");
        return;
    }

    // Send the signal to the process
    if (kill(pid, actual_signal) == 0) {
        printf("Sent signal %d to process with pid %d\n", actual_signal, pid);
    } else {
        perror("Failed to send signal");
    }
}

void fg_process(pid_t pid) {
    // Check if the process exists in the background list
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            // Bring process to the foreground
            foreground_pid = pid;
            strcpy(bg_processes[i].state, "Running"); // Update the state to Running
            kill(pid, SIGCONT); // Continue the process in the foreground
            printf("Process with PID %d has been brought to the foreground.\n", pid);
            waitpid(pid, NULL, 0); // Wait for the process to finish
            return;
        }
    }
    fprintf(stderr, "No such process found\n");
}

void bg_process(pid_t pid) {
    // Check if the process exists in the background list
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid && strcmp(bg_processes[i].state, "Stopped") == 0) {
            strcpy(bg_processes[i].state, "Running"); // Update the state
            kill(pid, SIGCONT); // Continue the process in the background
            printf("Process with PID %d is now running in the background.\n", pid);
            return;
        }
    }
    fprintf(stderr, "No such process found or process is not stopped.\n");
}
