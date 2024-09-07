#include "utils.h"
#include "commands.h"
#include "prompt.h"

ProcessInfo bg_processes[MAX_BG_PROCESSES]; // MAX_BG_PROCESSES is defined
int bg_count = 0; // Initialize bg_count
char* shell_home_directory;

// Function to handle SIGCHLD for background processes
void handle_sigchld(int sig) {
    int saved_errno = errno; // Save errno to restore later
    while (1) {
        pid_t pid = waitpid(-1, NULL, WNOHANG); // Non-blocking wait
        if (pid <= 0) break; // No more processes to wait for

        // Handling of finished background processes
        printf("Background process with PID %d exited.\n", pid);
        
        // Update the status of the terminated process
        for (int i = 0; i < bg_count; i++) {
            if (bg_processes[i].pid == pid) {
                strcpy(bg_processes[i].state, "Terminated"); // Mark as terminated
                break; // Break once found and updated
            }
        }
    }
    errno = saved_errno; // Restore errno
}

// Function to add a background process
void add_background_process(pid_t pid, const char *command) {
    if (bg_count < MAX_BG_PROCESSES) {
        bg_processes[bg_count].pid = pid;
        bg_processes[bg_count].command = strdup(command); // Duplicate the command string
        strcpy(bg_processes[bg_count].state, "Running");
        bg_count++;
    } else {
        fprintf(stderr, "Maximum number of background processes reached.\n");
    }
}

// Function to clean up background processes
void cleanup_bg_processes() {
    for (int i = 0; i < bg_count; i++) {
        free(bg_processes[i].command); // Free the allocated command string
    }
}

int main() {
    initialize_shell_home_directory(); 
    char command[4096];

    // Initialize global variables
    bg_count = 0; // Ensure to initialize the counter
    init_log(); 

    // Signal handler for SIGCHLD to handle background processes 

    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR) {
        perror("Failed to set signal handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Remove trailing newline characters
            command[strcspn(command, "\n")] = 0;

            if (strcmp(command, "exit") == 0) { // Self-defined exit command
                break;
            }
            add_to_log(command);  // Enter the command as it is in the log
            process_command(command);  // Main shell logic
        } else {
            if (feof(stdin)) {
                break; // End of input (Ctrl+D)
            } else {
                perror("Error reading command");
            }
        }
    }
    
    cleanup_bg_processes(); // Clean up allocated memory for background processes
    cleanup_log(); // Save the log at shutdown
    return 0;
}
