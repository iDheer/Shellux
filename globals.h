#ifndef GLOBALS_H
#define GLOBALS_H

#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <linux/limits.h>
// ---------------------------------------------------------------------------------------------
#define RESET "\033[0m"
#define BLUE "\033[1;34m" 
#define GREEN "\033[1;32m"
#define COLOR_RED "\033[31m"      // Red color
#define COLOR_RESET "\033[0m"     // Reset color
#define COLOR_BOLD_TEAL "\033[1;38;2;0;128;128m"   // Bold Teal
#define COLOR_BOLD_CORAL "\033[1;38;2;255;127;80m" // Bold Coral
// ---------------------------------------------------------------------------------------------
#define PORT "80"
#define MAX_PATH 4096
#define MAX_LOG_SIZE 15
#define MAX_ALIASES 100
#define BUFFER_SIZE 4096
#define MAX_COMMANDS 100
#define MAX_RESULTS 1000
#define HOST "man.he.net"
#define MAX_FUNCTIONS 100
#define MAX_BG_PROCESSES 100 
#define MAX_COLORED_PATH 5016
#define LOG_FILE_PATH "command_log.txt"
// ---------------------------------------------------------------------------------------------
typedef struct {
    pid_t pid;              
    char *command;           
    char state[32];          
} ProcessInfo;

typedef struct {
    char *alias_name;
    char *command;
} Alias;

typedef struct {
    char *func_name;
    char *func_body;  
} Function;
// ---------------------------------------------------------------------------------------------
extern int bg_count;               
extern int terminal_fd;                   
extern int alias_count;
extern int function_count;
extern pid_t foreground_pid;                 
extern char *shell_home_directory;       
extern Alias aliases[MAX_ALIASES];
extern Function functions[MAX_FUNCTIONS];
extern ProcessInfo bg_processes[MAX_BG_PROCESSES]; 
// ---------------------------------------------------------------------------------------------
#endif 
