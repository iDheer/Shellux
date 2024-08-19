#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/utsname.h>
#include "utils.h"

char *shell_home_directory; // Define the global variable

void initialize_shell_home_directory() {
    shell_home_directory = getcwd(NULL, 0); // Get the current working directory
}

// Get the current username
char* get_username() {
    struct passwd *pw = getpwuid(getuid());
    return pw->pw_name;
}

// Get the system name
char* get_system_name() {
    static struct utsname unameData;
    uname(&unameData);
    return unameData.nodename;
}

// Check if the current directory is the home directory
int is_home_directory(const char *cwd) {
    return strcmp(cwd, shell_home_directory) == 0;
}
