#ifndef GLOBALS_H
#define GLOBALS_H

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdbool.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/wait.h>
#include <pwd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <libgen.h>
#include <ctype.h>

// Constants for maximum sizes
#define BUFFER_SIZE 4096
#define HOST "man.he.net"
#define PORT "80"
#define MAX_PATH 4096
#define MAX_RESULTS 1000
#define LOG_FILE_PATH "command_log.txt"
#define MAX_LOG_SIZE 15
#define MAX_COLORED_PATH 5016
#define MAX_COMMANDS 100
#define MAX_BG_PROCESSES 100 // Maximum number of background processes

// Definition of the ProcessInfo structure
typedef struct {
    pid_t pid;               // Process ID
    char *command;           // Command associated with the process
    char state[32];          // Current state: "Running", "Stopped", or "Terminated"
} ProcessInfo;

// Declaration of global variables
extern pid_t foreground_pid;                   // PID of the current foreground process
extern int bg_count;                // Counter for background processes
extern ProcessInfo bg_processes[MAX_BG_PROCESSES]; // Array of ProcessInfo structures
extern char *shell_home_directory;         // Global variable to store the shell's home directory
extern int terminal_fd;                    // File descriptor for the terminal

#endif 
