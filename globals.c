#include "utils.h"
#include "commands.h"
#include "prompt.h"
#include "alias.h"

int bg_count = 0;
int log_count = 0;
int alias_count = 0;
pid_t foreground_pid = -1; 
char* shell_home_directory;
static volatile int running = 1; 
static Function *function_list = NULL;

Alias aliases[MAX_ALIASES];
char *command_log[MAX_LOG_SIZE];
char prev_dir[PATH_MAX] = ""; 
ProcessInfo bg_processes[MAX_BG_PROCESSES];

void handle_error(const char *message) {
    fprintf(stderr, COLOR_RED "Error: %s: %s\n" COLOR_RESET, message, strerror(errno));
}

void update_foreground_pid(pid_t pid) {
    foreground_pid = pid;
}

int is_background_process(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            return 1; 
        }
    }
    return 0;
}

void cleanup_bg_processes() {
    for (int i = 0; i < bg_count; i++) {
        free(bg_processes[i].command); 
        bg_processes[i].command = NULL; // Avoid dangling pointers
    }
    bg_count = 0; 
}

void remove_background_process(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            free(bg_processes[i].command); 
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_count--;
            // Only set the last command to NULL if there are remaining processes
            if (bg_count > 0) {
                bg_processes[bg_count].command = NULL; // Set last command to NULL
            }
            break;
        }
    }
}

void add_to_background_processes(pid_t pid, char *command) {
    if (bg_count < MAX_BG_PROCESSES) {
        bg_processes[bg_count].pid = pid;
        bg_processes[bg_count].command = strdup(command);
        if (bg_processes[bg_count].command == NULL) {
            handle_error("Failed to allocate memory for background process command");
            exit(EXIT_FAILURE);
        }
        // strcpy(bg_processes[bg_count].state, state);
        bg_count++;
    } else {
        purge_oldest_background_commands(); // Remove the oldest background process
        add_to_background_processes(pid, command); // Try again
    }
}

// Purge the oldest background commands from the global array
void purge_oldest_background_commands() {
    int purge_count = MAX_BG_PROCESSES / 2; // Purge the oldest 50 commands

    // Shift remaining commands up by 50
    for (int i = purge_count; i < bg_count; i++) {
        bg_processes[i - purge_count] = bg_processes[i];
    }

    // Reduce the background process count
    bg_count -= purge_count;
}

// Trim leading and trailing whitespace for alias processing
char* trim_whitespace_a(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    end[1] = '\0';

    return str;
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

char* get_command_name(pid_t pid) {
    char path[40];
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    
    FILE *file = fopen(path, "r");
    if (!file) {
        handle_error("Failed to open /proc/pid/comm file");
        return NULL;
    }

    char *command = malloc(256); // Allocate memory for the command name
    if (fgets(command, 256, file) != NULL) {
        // Remove the newline character at the end
        command[strcspn(command, "\n")] = 0;
    }
    fclose(file);
    return command;
}

// Initialize the shell home directory
void initialize_shell_home_directory() {
    shell_home_directory = getcwd(NULL, 0); // Retrieve current working directory
    if (!shell_home_directory) {
        handle_error("Failed to get current working directory");
        exit(EXIT_FAILURE);
    }
}

// Check if the current directory is the home directory
int is_home_directory(const char *cwd) {
    return strcmp(cwd, shell_home_directory) == 0;
}


