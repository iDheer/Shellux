#include "utils.h"
#include "commands.h"
#include "prompt.h"

void display_prompt() {
    
    char cwd[4096];
    getcwd(cwd, sizeof(cwd)); // Get the current working directory and store it in the cwd buffer

    char *username = get_username();
    char *sysname = get_system_name();

    char display_path[4096];

    // Check if current directory is home directory
    if (is_home_directory(cwd)) {
        snprintf(display_path, sizeof(display_path), "~");
    } else {
        // Check if current directory is inside the home directory
        if (strncmp(cwd, shell_home_directory, strlen(shell_home_directory)) == 0) {
            snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(shell_home_directory));
        } else {
            snprintf(display_path, sizeof(display_path), "%s", cwd);
        }
    }

    // Print < in bold coral, username, @ in bold coral, sysname in bold coral, : in normal, and path in bold teal, and > in bold teal
    printf(COLOR_BOLD_TEAL "<" COLOR_BOLD_TEAL "%s" COLOR_BOLD_TEAL "@" "%s" COLOR_RESET ":" COLOR_BOLD_CORAL "%s" COLOR_BOLD_CORAL ">" COLOR_RESET " ", username, sysname, display_path);
    fflush(stdout);
}
