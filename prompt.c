#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"
#include "prompt.h"

// Display the prompt
void display_prompt() {
    char cwd[1024];
    char *home_dir = getenv("HOME");
    getcwd(cwd, sizeof(cwd));  // Get the current working directory

    char *username = get_username();
    char *sysname = get_system_name();
    char display_path[1024];

    if (is_home_directory(cwd)) {
        snprintf(display_path, sizeof(display_path), "~");
    } else {
        // Check if the current directory is within the home directory
        if (strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
            snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(home_dir));
        } else {
            snprintf(display_path, sizeof(display_path), "%s", cwd);
        }
    }

    printf("<%s@%s:%s> ", username, sysname, display_path);
    fflush(stdout);
}
