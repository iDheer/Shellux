#include "utils.h"
#include "commands.h"
#include "prompt.h"
#include <signal.h>  // Include for signal handling

ProcessInfo bg_processes[MAX_BG_PROCESSES]; // MAX_BG_PROCESSES is defined
int bg_count = 0; // Initialize bg_count
char* shell_home_directory;
pid_t foreground_pid = -1; // Global variable to track the foreground process ID

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

// void handle_sigint(int sig) {
//     if (foreground_pid > 0) {
//         // Send SIGINT to the foreground process
//         kill(foreground_pid, SIGINT);
//     }
// }

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

// void handle_sigtstp(int sig) {
//     if (foreground_pid > 0) {
//         printf("Sending SIGTSTP to process with PID %d\n", foreground_pid);
//         // Send SIGTSTP to stop the foreground process
//         kill(foreground_pid, SIGTSTP);
        
//         // Update the status of the stopped process
//         add_background_process(foreground_pid, "foreground process");

//         for(int i = 0; i < bg_count; i++) {
//             if (bg_processes[i].pid == foreground_pid) {
//                 strcpy(bg_processes[i].state, "Stopped"); // Mark as stopped
//                 break; // Break once found and updated
//             }
//         }

//         // Add the stopped process to the background list

//         printf("Process with PID %d has been moved to the background and stopped.\n", foreground_pid);
//         foreground_pid = -1; // Reset foreground PID
//         tcsetpgrp(STDIN_FILENO, getpid()); // Regain control of terminal
//     }
// }

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

    // Set signal handlers
    if (signal(SIGCHLD, handle_sigchld) == SIG_ERR) {
        perror("Failed to set SIGCHLD handler");
        exit(EXIT_FAILURE);
    }

    // if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    //     perror("Failed to set SIGINT handler");
    //     exit(EXIT_FAILURE);
    // }

    // if (signal(SIGTSTP, handle_sigtstp) == SIG_ERR) {
    //     perror("Failed to set SIGTSTP handler");
    //     exit(EXIT_FAILURE);
    // }

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
                // Log out and kill all background processes
                for (int i = 0; i < bg_count; i++) {
                    kill(bg_processes[i].pid, SIGTERM); // Send SIGTERM to background processes
                }
                break; // Exit the shell
            } else {
                perror("Error reading command");
            }
        }
    }
    
    cleanup_bg_processes(); // Clean up allocated memory for background processes
    cleanup_log(); // Save the log at shutdown
    return 0;
}
