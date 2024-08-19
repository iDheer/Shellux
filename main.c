#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include "commands.h"
#include "prompt.h"

int main() {
    signal(SIGCHLD, handle_sigchld); // Handle background process termination, this calls the handle_sigchld function when it receives the SIGCHLD signal
    signal(SIGINT, handle_sigint);   // Handle Ctrl+C, this calls the handle_sigint function when it receives the SIGINT signal
    signal(SIGTSTP, handle_sigtstp); // Handle Ctrl+Z, this calls the handle_sigtstp function when it receives the SIGTSTP signal

    initialize_shell_home_directory(); // Initialize shell home directory
    char command[1024];

    while (1) {
        display_prompt();  // Display the shell prompt
        if (fgets(command, sizeof(command), stdin) != NULL) { 
            process_command(command);  // Process the entered command
        }
    }
    
    return 0;
}
