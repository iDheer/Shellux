#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "commands.h"
#include "prompt.h"

int main() {
    char command[1024];

    while (1) {
        display_prompt();  // Display the shell prompt
        if (fgets(command, sizeof(command), stdin) != NULL) { 
            process_command(command);  // Process the entered command
        }
    }
    
    return 0;
}
