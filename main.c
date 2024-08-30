#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include "prompt.h"
#include "commands.h"

int main() {
    initialize_shell_home_directory(); 
    char command[4096];

    init_log();  // Initialize and load the log at startup

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) { 
            // Remove trailing newline character if present

            command[strcspn(command, "\n")] = 0;

            if (strcmp(command, "exit") == 0) { // self defined exit command
                break;
            }
            
            add_to_log(command);  // Log the command after processing
            process_command(command);  // Process the command first
            
        }
    }
    cleanup_log();  // Clean up and save the log at shutdown
    return 0;
}
