# OSN Mini Project 1

**Author**: Inesh Dheer  
**Roll Number**: 2023111010  
**Branch**: CSD  

## main.c

This project implements a simple command-line shell in C that allows users to execute commands, manage background processes, and maintain a command log. The shell is designed to provide a user-friendly interface and includes features for handling various commands and signals.

## Table of Contents

- [Features](#features)
- [Usage](#usage)
- [Assumptions](#assumptions)
- [Signal Handling](#signal-handling)
- [Functionality](#functionality)
- [Command Logging](#command-logging)
- [Error Handling](#error-handling)
- [Dependencies](#dependencies)
- [License](#license)

## Features

- **Command Execution**: Execute shell commands entered by the user.
- **Background Process Management**: Handles background processes and notifies the user when they exit.
- **Command Logging**: Logs all executed commands for later reference.
- **Signal Handling**: Properly handles termination signals for child processes.

## Usage

To run the shell, compile the code and execute the resulting binary. The shell will prompt the user for commands. The user can enter any valid shell command, and the shell will attempt to execute it.

### Exiting the Shell

To exit the shell, type the command `exit`. This will terminate the shell session and trigger the cleanup of resources, including saving the command log.

## Assumptions

- The user must type `exit` to terminate the shell. This command is necessary to ensure that the command log is saved properly before the shell exits.
- Commands entered into the shell are assumed to be valid. Invalid commands will be handled by the underlying operating system.
- The shell operates in a standard POSIX environment and relies on standard C libraries.
- Input commands are limited to a maximum length of 4096 characters.
- The shell does not support advanced features like command piping or redirection in this implementation. Only basic command execution is available.

## Signal Handling

The shell utilizes the `SIGCHLD` signal to handle the termination of child processes. This signal is caught, and the shell waits for any child processes that have exited. When a background process finishes, the shell outputs a notification message indicating the process ID (PID) of the exited process.

## prompt.c

The `prompt.c` file contains the implementation of the command prompt displayed to the user in the shell. It provides a user-friendly interface that includes the current username, system name, and the current working directory. This information helps users to identify their location in the file system and the context of their shell session.

### Functionality

The primary function in this file is `display_prompt()`, which constructs and displays the command prompt. The following steps are performed in this function:

1. **Get Current Working Directory**: 
   - The shell retrieves the current working directory using the `getcwd` function and stores it in a buffer for later use.

   ```c
   char cwd[4096];
   getcwd(cwd, sizeof(cwd));

```c

## commands.c

### Overview
The `commands.c` file contains the core functionalities of the custom shell, including command execution, process management, and environment handling. This file serves as a bridge between user inputs and the system's command execution capabilities, allowing for both foreground and background processes.

### Key Functionalities
- **Shell Home Directory Initialization**: Retrieves and stores the shell's home directory at startup.
- **User and System Information**: Functions to obtain the current username and system name.
- **Command Execution**: Manages the execution of commands, both built-in and external, with proper handling of command-line arguments.
- **Background Process Management**: Supports background command execution and tracks the associated process IDs (PIDs).

### Functions

1. **`void initialize_shell_home_directory()`**:
   - Initializes the global variable `shell_home_directory` with the current working directory using `getcwd`.
   - Handles errors during initialization by exiting with an error message.

2. **`char* get_username()`**:
   - Retrieves the current user's username using `getpwuid`.
   - Returns "unknown" if the username cannot be retrieved.

3. **`char* get_system_name()`**:
   - Retrieves the system's name using `uname`.
   - Returns the node name of the system.

4. **`int is_home_directory(const char *cwd)`**:
   - Checks if the current working directory (`cwd`) matches the shell's home directory.
   - Returns `1` if true; otherwise, returns `0`.

5. **`void execute_command(char *cmd)`**:
   - Tokenizes the input command string into arguments.
   - Logs the command for tracking.
   - Handles built-in commands such as `hop`, `reveal`, `log`, and `proclore`.
   - Executes external commands using `fork` and `execvp`.
   - Measures execution time for foreground processes and displays it if the process takes more than 2 seconds.

6. **`void process_command(char *input)`**:
   - Processes multiple commands separated by semicolons (`;`).
   - Supports background execution using the `&` symbol.
   - Manages the execution of each command, determining if it should run in the foreground or background.

### Background Process Management
- Up to `MAX_BG_PROCESSES` (1024) background processes can be tracked.
- The arrays `bg_pids` and `bg_commands` store the PIDs and commands of background processes.
- Background processes are executed by forking a new process and allowing the parent to continue without waiting.

### Assumptions
- The user must type `exit` to terminate the shell; this will gracefully shut down the program and log the commands.
- All commands are expected to be entered in a format that adheres to standard shell syntax.
- Proper permissions are required for executing certain commands and accessing system resources.

### Example Usage
Here is an example of how the shell prompt might behave:

```bash
<IneshDheer@MyPC:~> ls -la &
Background PID: 12345
<IneshDheer@MyPC:~> echo "Hello, World!"
Hello, World!
<IneshDheer@MyPC:~> exit
```

## utils.c

### Overview
The `utils.c` file provides various utility functions that support the functionality of the custom shell. This includes functions for user and system information retrieval, as well as handling string manipulations that are crucial for command processing and execution.

### Key Functionalities
- **User and System Information**: Functions to retrieve the current user's name and the system's name.
- **String and Path Utilities**: Utility functions for handling paths and checking directory types.
- **Error Handling**: Centralized error reporting mechanisms for better maintainability.

### Functions

1. **`char* get_username()`**:
   - Retrieves the username of the currently logged-in user using the `getpwuid` function.
   - Returns "unknown" if the username cannot be determined.

2. **`char* get_system_name()`**:
   - Uses the `uname` function to obtain the system's node name (hostname).
   - Returns the hostname as a string.

3. **`int is_home_directory(const char *cwd)`**:
   - Checks if the provided current working directory (`cwd`) matches the shell's home directory.
   - Returns `1` if it matches; otherwise, returns `0`.

4. **`void handle_error(const char *msg)`**:
   - Centralized error handling function that prints the provided error message to `stderr`.
   - Uses `perror` to display the associated system error message based on `errno`.

5. **`char* get_current_directory()`**:
   - Retrieves the current working directory using `getcwd`.
   - Returns a dynamically allocated string containing the directory path.

6. **`void free_string_array(char **array)`**:
   - Frees a dynamically allocated array of strings.
   - Iterates through the array, freeing each string and then the array itself.

### Example Usage
Here is an example of how the utility functions might be used in conjunction with the shell:

```c
#include "utils.h"

char *username = get_username();
printf("Current user: %s\n", username);

char *cwd = get_current_directory();
printf("Current directory: %s\n", cwd);
free(cwd); // Remember to free the allocated memory


## Header Files

### prompt.h

The `prompt.h` header file defines the interface for displaying the command prompt in the shell.

#### Functions

- **`void display_prompt();`**
  - Displays the command prompt, including the current user's username, system name, and the current working directory.

### commands.h

The `commands.h` header file declares various command-related functions used in the shell, including built-in commands for logging and directory navigation.

#### Constants
- **`#define MAX_PATH 4096`**: Maximum path length supported by the shell.
- **`#define MAX_RESULTS 1000`**: Maximum number of results returned from search operations.

#### Functions

- **`void save_log();`**
  - Saves the command log to persistent storage.

- **`void init_log();`**
  - Initializes the command log.

- **`void clear_log();`**
  - Clears the current command log.

- **`void cleanup_log();`**
  - Cleans up resources related to the command log.

- **`void hop(char **args, int argc);`**
  - Implements the 'hop' command, which changes the current directory.

- **`void seek(char **args, int argc);`**
  - Implements the 'seek' command, which searches for files in a directory.

- **`void reveal(char **args, int argc);`**
  - Reveals hidden files or directories based on the provided arguments.

- **`void proclore(char **args, int argc);`**
  - Implements the 'proclore' command, which displays process information.

- **`void add_to_log(const char *command);`**
  - Adds a command to the log.

- **`void log_command(char **args, int argc);`**
  - Logs the executed command.

- **`void add_colored_path(char *dest, const char *path, const char *color_code);`**
  - Adds a path with a specified color code to the display.

- **`void search_directory(const char *directory, const char *search_term, char results[MAX_RESULTS][MAX_PATH], int *result_count, int d_flag, int f_flag);`**
  - Searches a specified directory for files matching the search term and populates the results array.

### utils.h

The `utils.h` header file declares utility functions that provide various system and user-related functionalities in the shell.

#### Global Variables

- **`extern char *shell_home_directory;`**
  - Global variable that stores the shell's home directory.

#### Functions

- **`char* get_username();`**
  - Retrieves the current username.

- **`char* get_system_name();`**
  - Retrieves the system's hostname.

- **`void execute_command(char *cmd);`**
  - Executes a given command, handling both built-in and external commands.

- **`void process_command(char *input);`**
  - Processes user input and executes commands separated by semicolons.

- **`void initialize_shell_home_directory();`**
  - Initializes the shell's home directory variable.

- **`int is_home_directory(const char *cwd);`**
  - Checks if the specified current working directory is the home directory.

- **`void reveal(char **args, int argc);`**
  - Reveals hidden files or directories based on provided arguments.

- **`void log_command(char **args, int argc);`**
  - Logs the executed command.

- **`void add_to_log(const char *command);`**
  - Adds a command to the log.

### Conclusion

These header files provide the necessary declarations for the various functionalities implemented in the shell. They enable modular development and ease of maintenance by separating the interface from the implementation.
