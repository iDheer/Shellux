#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/utsname.h>
#include "utils.h"

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
    char *home = getenv("HOME");
    return strcmp(cwd, home) == 0;
}
