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

    init_log();

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) { 
            if (strcmp(command, "exit\n") == 0) { // slef defined exit command
                break;
            }
            process_command(command);  
        }
    }
    cleanup_log();
    return 0;
}
