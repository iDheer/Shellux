#include "utils.h"
#include "commands.h"
#include "prompt.h"

void display_prompt() {
    
    char cwd[4096];
    getcwd(cwd, sizeof(cwd)); // Get the current working directory and store it in the cwd buffer

    char *username = get_username();
    char *sysname = get_system_name();

    char display_path[4096];

    // Pehle check karo ki current directory home directory hai ya nahi
    if (is_home_directory(cwd)) {
        snprintf(display_path, sizeof(display_path), "~"); // yeh bas string ko display ke buffer mein store kar deta, print karne ke liye printf ka use karna padega
    } else {
        // Ab agar home directory nahi hai toh check karo ki current directory home directory ke andar hai ya nahi
        if (strncmp(cwd, shell_home_directory, strlen(shell_home_directory)) == 0) { // to know ki kya current directory home directory ke andar hai ya nahi, cause agar hai toh intial string will be same as the home directory uske baad extra characters honge
            snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(shell_home_directory));
                // This is the format string used by snprintf. The %s placeholder will be replaced by the string argument following it.
                // This expression calculates the address of the substring of cwd that starts right after shell_home_directory. This effectively gives the relative path from the home directory.
        } else {
            snprintf(display_path, sizeof(display_path), "%s", cwd);
        }
    }

    printf("<%s@%s:%s> ", username, sysname, display_path);
    fflush(stdout);
}