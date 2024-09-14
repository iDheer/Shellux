#include "utils.h"
#include "commands.h"
#include "prompt.h"
#include "alias.h"

void handle_sigchld(int sig) {
    int saved_errno = errno;
    int status;
    pid_t pid;

    // Wait for exited or signaled children only
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            // Check if the PID is a background process
            if (is_background_process(pid)) {
                printf("Background process with PID %d exited.\n", pid);
                remove_background_process(pid);
            } else if (pid == foreground_pid) {
                // Handle the case where the foreground process has exited
                printf("Foreground process with PID %d exited.\n", pid);
                update_foreground_pid(-1);
            }
        }
    }
    errno = saved_errno;
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
    initialize_shell_home_directory(); 
    init_log(); 
    load_myshrc();
    handle_sigchld(SIGCHLD);

    // Set signal handlers for SIGINT and SIGTSTP

    struct sigaction sa;  // Declare sa once

    // Set up signal handler for SIGINT (Ctrl + C)
    sa.sa_handler = custom_handler;  // Assign custom handler
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // No flags for now

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting SIGINT handler");
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for SIGTSTP (Ctrl + Z)
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Error setting SIGTSTP handler");
        exit(EXIT_FAILURE);
    }

    // Set up signal handler for SIGCHLD (for background process cleanup)
    sa.sa_handler = handle_sigchld;  // Use another handler for SIGCHLD
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Restart syscalls if interrupted by handler

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error setting SIGCHLD handler");
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
                    if (kill(bg_processes[i].pid, SIGKILL) == -1) {
                        if(waitpid(bg_processes[i].pid, 0, WNOHANG)==0){
                            sleep(1);
                        } // Wait for the process to terminate
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
