#include "utils.h"
#include "commands.h"
#include "prompt.h"

static char prev_dir[PATH_MAX] = ""; // Global variable for previous directory
static volatile int running = 1;  // Flag to control the loop
char *command_log[MAX_LOG_SIZE];
int log_count = 0;

void add_to_background_processes(pid_t pid, char *command);

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
    if (argc != 2) {
        printf("Usage: proclore <pid>\n");
        return;
    }
    // extract pid from args and argc
    int pid = atoi(args[1]);
    if (pid <= 0) {
        printf("Invalid PID\n");
        return;
    } 

    char path[4096];
    char status[4096];
    char state[4096];
    char exec_path[4096];
    long int vmsize;

    sprintf(path, "/proc/%d/status", pid);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }
    //print pid
    printf("Pid: %d\n",pid);

    //print process group
    pid_t pgid = getpgid(pid);
    if (pgid < 0) {
        perror("getpgid");
        return;
    }
    printf("Process Group: %d\n", pgid);
    //print state and vmsize using status file
    while (fgets(status, sizeof(status), fp) != NULL) {
        
         if (strncmp(status, "State:", 6) == 0) {
           if (pgid == pid &&(status[7]=='R' || status[7]=='S')) {
            printf("State: %c+\n",status[7]);
            } else {
              printf("State: %c\n",status[7]);
            }
        }
        else if (strncmp(status, "VmSize:", 7) == 0) {
            printf("%s", status);
        }
    }

    fclose(fp);
   
    //print executable path
    sprintf(path, "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exec_path, sizeof(exec_path) - 1);
    if (len != -1) {
        exec_path[len] = '\0';
        printf("Executable Path: %s\n", exec_path);
    } else {
        perror("Ececutable path");
    }
    
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

// Function to handle exit signals
void handle_exit(int sig) {
    running = 0;  // Set flag to stop printing PIDs
}

// Function to set the terminal to raw mode
void set_raw_mode(struct termios *orig_termios) {
    struct termios new_termios;

    // Get the current terminal attributes
    tcgetattr(STDIN_FILENO, orig_termios);
    new_termios = *orig_termios;

    // Modify the terminal attributes for raw mode
    new_termios.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    new_termios.c_cc[VMIN] = 1;              // Minimum of 1 character
    new_termios.c_cc[VTIME] = 0;             // No timeout

    // Set the terminal attributes to raw mode
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

// Function to reset the terminal to original mode
void reset_terminal(struct termios *orig_termios) {
    // Restore the original terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, orig_termios);
}

// Function to print the most recent PID
void print_latest_pid() {
    pid_t pid = fork();  // Fork a child to get the last PID
    if (pid == 0) {
        // In the child process, execute a command to get the last PID
        FILE *fp = popen("pgrep -n .", "r"); // Use pgrep to get the most recent PID
        if (fp == NULL) {
            perror("popen");
            exit(EXIT_FAILURE);
        }

        char buffer[128];
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s", buffer);  // Print the PID
        }
        pclose(fp);
        exit(EXIT_SUCCESS);  // Exit child process
    } else if (pid > 0) {
        wait(NULL);  // Wait for the child process to finish
    } else {
        perror("fork");
        exit(EXIT_FAILURE);
    }
}

char kbhit() {
    struct termios oldt, newt;
    int oldf;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    int result = read(STDIN_FILENO, &ch, 1); // Read a single character
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf); // Restore the original flags

    if (result > 0) {
        return ch; // Return the character read
    }

    return '\0'; // No character was read
}

void neonate(int time_arg) {
    struct termios orig_termios;
    set_raw_mode(&orig_termios); // Set terminal to raw mode
    signal(SIGINT, handle_exit);  // Catch Ctrl+C to exit
    signal(SIGQUIT, handle_exit);  // Catch Ctrl+\ to exit

    // Loop to print the PID every time_arg seconds
    while (running) {
        print_latest_pid(); // Print the latest PID
        sleep(time_arg);    // Wait for specified time

        // Check for user input
        char key = kbhit();
        if (key == 'x') {
            printf("\nYou pressed 'x'. Exiting...\n");
            running = 0;  // Stop the loop if 'x' is pressed
        }
    }

    reset_terminal(&orig_termios); // Restore original terminal mode
    printf("\nExiting neonate command.\n");
}

void iMan(char *cmd) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(HOST, PORT, &hints, &servinfo) != 0) {
        perror("getaddrinfo");
        return;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;  // Try the next address
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;  // Try the next address
        }

        break;  // Successfully connected
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to connect\n");
        return;
    }

    freeaddrinfo(servinfo);  // Free the linked list

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request),
             "GET /?topic=%s&section=all HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n",
             cmd, HOST);

    if (send(sockfd, request, strlen(request), 0) == -1) {
        perror("send");
        close(sockfd);
        return;
    }

    int header_ended = 0;
    while ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';

        printf("%s", buffer);  // Print the rest of the response
    }

    if (bytes_received == -1) {
        perror("recv");
    }

    close(sockfd);  // Close the socket
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
    int found = 0;

    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            found = 1;
            foreground_pid = pid;
            strcpy(bg_processes[i].state, "Running");

            printf("Giving terminal control to PID %d\n", pid);

            // Remove the process from the background list
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_count--;

            kill(pid, SIGCONT); // Continue the process if it was stopped
            printf("Process with PID %d has been brought to the foreground.\n", pid);

            int status;
            while (1) {
                // Wait for the process state to change
                pid_t result = waitpid(pid, &status, WUNTRACED | WCONTINUED);

                if (result == -1) {
                    perror("waitpid failed");
                    break;
                }

                if (WIFEXITED(status)) {
                    // Process has exited normally
                    printf("Process with PID %d has exited.\n", pid);
                    break;
                }

                if (WIFSIGNALED(status)) {
                    // Process was killed by a signal
                    printf("Process with PID %d was killed by signal %d.\n", pid, WTERMSIG(status));
                    break;
                }

                if (WIFSTOPPED(status)) {
                    // Process was stopped (e.g., via Ctrl-Z)
                    printf("Process with PID %d was stopped.\n", pid);
                    add_to_background_processes(pid, get_command_name(pid));
                    break;
                }

                if (WIFCONTINUED(status)) {
                    // Process was resumed (e.g., via SIGCONT)
                    printf("Process with PID %d was continued.\n", pid);
                }
            }

            // Reset the foreground PID
            foreground_pid = -1;
            return;
        }
    }

    if (!found) {
        fprintf(stderr, "No such process found with PID %d\n", pid);
    }
}

void bg_process(pid_t pid) {
    // Check if the process exists in the background list
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            // check if it was a stopped process
            if (strcmp(bg_processes[i].state, "Stopped") == 0) {
                ping_process(pid, 18);
                activities();
                // Send SIGCONT signal to continue the process
                printf("Continuing process with PID %d\n", pid);
            } else {
                printf("Process with PID %d is already running\n", pid);
            }
            return;
        }
    }
    printf("No such process found with PID %d\n", pid);
}
