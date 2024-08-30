#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"
#include "commands.h"
#include "prompt.h"

// Function to handle SIGCHLD
void handle_sigchld(int sig) {
    int saved_errno = errno; // Save errno to restore later
    while (1) {
        pid_t pid = waitpid(-1, NULL, WNOHANG); // Non-blocking wait
        if (pid <= 0) break; // No more processes to wait for
        // Handle finished background processes
        printf("Background process with PID %d exited.\n", pid);
    }
    errno = saved_errno; // Restore errno
}

int main() {
    initialize_shell_home_directory(); 
    char command[4096];

    init_log();  // Initialize and load the log at startup

    // Set up signal handler for SIGCHLD
    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR) {
        perror("Failed to set signal handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // Remove trailing newline character if present
            command[strcspn(command, "\n")] = 0;

            if (strcmp(command, "exit") == 0) { // self-defined exit command
                break;
            }
            add_to_log(command);  // Log the command after processing
            process_command(command);  // Process the command
        } else {
            if (feof(stdin)) {
                break; // End of input (Ctrl+D)
            } else {
                perror("Error reading command");
            }
        }
    }
    
    cleanup_log();  // Clean up and save the log at shutdown
    return 0;
}
