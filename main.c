#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include "prompt.h"

int main() {
    initialize_shell_home_directory(); 
    char command[4096];

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) { 
            process_command(command);  
        }
    }
    
    return 0;
}
