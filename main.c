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

// handling the SIGCHLD
void handle_sigchld(int sig) {
    int saved_errno = errno; // Save errno to restore later
    while (1) {
        pid_t pid = waitpid(-1, NULL, WNOHANG); // Non-blocking wait
        if (pid <= 0) break; // No more processes to wait for
        // Handling of finished background processes
        printf("Background process with PID %d exited.\n", pid);
    }
    errno = saved_errno; // Restore errno
}

int main() {
    initialize_shell_home_directory(); 
    char command[4096];

    init_log(); 

    // Setting up signal handler for SIGCHLD
    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR) {
        perror("Failed to set signal handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) {
            // tailing newline characters hatane ke liye
            command[strcspn(command, "\n")] = 0;

            if (strcmp(command, "exit") == 0) { // self-defined exit command
                break;
            }
            add_to_log(command);  // enter the command as it is in the log
            process_command(command);  // main shell logic
        } else {
            if (feof(stdin)) {
                break; // End of input (Ctrl+D)
            } else {
                perror("Error reading command");
            }
        }
    }
    
    cleanup_log(); // save the log at shutdown
    return 0;
}
