#include "utils.h"
#include "commands.h"
#include "prompt.h"

int bg_count = 0;
int alias_count = 0;
int function_count = 0;
pid_t foreground_pid = -1; 
char* shell_home_directory;
Alias aliases[MAX_ALIASES];
Function functions[MAX_FUNCTIONS];
ProcessInfo bg_processes[MAX_BG_PROCESSES];

void handle_error(const char *message) {
    fprintf(stderr, COLOR_RED "Error: %s: %s\n" COLOR_RESET, message, strerror(errno));
}

extern void cleanup_bg_processes() {
    for (int i = 0; i < bg_count; i++) {
        free(bg_processes[i].command); 
        bg_processes[i].command = NULL; // Avoid dangling pointers
    }
    bg_count = 0; 
}

char* trim_whitespace_a(char* str) {
    while (isspace((unsigned char)*str)) str++; // Trim leading whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--; // Trim trailing whitespace
    *(end + 1) = '\0'; // Null-terminate the trimmed string
    return str;
}

extern void load_myshrc() {
    FILE *file = fopen("inesh.myshrc", "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            // Process aliases
            if (strstr(line, "alias") != NULL) {
                char *alias_name = strtok(line + 6, "=");
                char *command = strtok(NULL, "\n");

                alias_name = trim_whitespace_a(alias_name);
                command = trim_whitespace_a(command);

                if (alias_name && command && alias_count < MAX_ALIASES) {
                    aliases[alias_count].alias_name = strdup(alias_name);
                    aliases[alias_count].command = strdup(command);
                    printf("Loaded alias: %s -> %s\n", alias_name, command);  // Debug
                    alias_count++;
                }
            }
        }
        fclose(file);
    }
}

extern void remove_background_process(pid_t pid) {
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

extern void add_to_background_processes(pid_t pid, char *command) {
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

void custom_handler(int signum) {
    if (signum == SIGINT) { // Ctrl + C
        if (foreground_pid > 0) {
            kill(foreground_pid, SIGINT); // Terminate foreground process
            printf("Terminated foreground process (PID: %d)\n", foreground_pid);
            update_foreground_pid(-1); // Reset foreground PID
        } else {
            printf("\nNo foreground process to terminate\n");
        }
    } 
    else if (signum == SIGTSTP) { // Ctrl + Z
        if (foreground_pid > 0) {
            kill(foreground_pid, SIGTSTP); // Stop foreground process
            printf("Stopped foreground process (PID: %d)\n", foreground_pid);
            add_to_background_processes(foreground_pid, get_command_name(foreground_pid));
            update_foreground_pid(-1); // Reset foreground PID
        } else {
            printf("\nNo foreground process to stop\n");
        }
    }
}

int main() {
    char command[4096];
    init_log(); 
    load_myshrc();
    initialize_shell_home_directory(); 

    // Set signal handlers for SIGINT and SIGTSTP
    struct sigaction sa;
    sa.sa_handler = custom_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        handle_error("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        handle_error("Failed to set SIGTSTP handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) {
            command[strcspn(command, "\n")] = 0; // Remove trailing newline characters

            if (strcmp(command, "exit") == 0) { // Self-defined exit command
                break;
            }
            add_to_log(command); // Enter the command in the log
            process_command(command); // Main shell logic
        } else {
            if (feof(stdin)) {
                // Log out and kill all background processes
                for (int i = 0; i < bg_count; i++) {
                    if (kill(bg_processes[i].pid, SIGTERM) == -1) {
                        handle_error("Failed to terminate background process");
                    }
                }
                break; // Exit the shell
            } else {
                continue; // Ignore empty input
            }
        }
    }

    cleanup_bg_processes(); // Clean up allocated memory for background processes
    cleanup_log(); // Save the log at shutdown
    return 0;
}
