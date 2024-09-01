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

## Features

- **Command Execution**: Execute shell commands entered by the user.
- **Command Logging**: Logs all executed commands for later reference.
- **Signal Handling**: Properly handles termination signals for child processes.
- **Background Process Management**: Handles background processes and notifies the user when they exit.

## Usage

To run the shell, compile the code and execute the resulting file. The shell will prompt the user for commands. The user can enter any valid shell command, and the shell will attempt to execute it. (right now for spec 1-8)

### Exiting the Shell

To exit the shell, type the command `exit`. This will terminate the shell session and trigger the cleanup of resources, including saving the command log.

## Assumptions

- The user must type `exit` to terminate the shell. This command is necessary to ensure that the command log is saved properly before the shell exits.
- Commands entered into the shell are assumed to be valid. Invalid commands will be handled by the underlying operating system.
- Input commands are limited to a maximum length of 4096 characters.
- The shell does not support advanced features like command piping or redirection in this implementation as of now.

## Signal Handling

The shell utilizes the `SIGCHLD` signal to handle the termination of child processes. This signal is caught, and the shell waits for any child processes that have exited. When a background process finishes, the shell outputs a notification message indicating the process ID (PID) of the exited process.

## prompt.c

The `prompt.c` file contains the implementation of the command prompt displayed to the user in the shell. It provides a user-friendly interface that includes the current username, system name, and the current working directory. This information helps users to identify their location in the file system and the context of their shell session.
The primary function in this file is `display_prompt()`, which constructs and displays the command prompt. The following steps are performed in this function:

## commands.c

### Overview

The `commands.c` file contains the core functionalities of the custom shell, including command execution, process management, and environment handling. This file serves as a bridge between user inputs and the system's command execution capabilities, allowing for both foreground and background processes, here the main user defined commands such as `hop`, `reveal`, `log`, 'proclore' and 'seek' are implemented.

### Functions

- **`void initialize_shell_home_directory()`**:
  - Initializes the global variable `shell_home_directory` with the current working directory using `getcwd`.
  - Handles errors during initialization by exiting with an error message.

- **`char* get_username()`**:
  - Retrieves the current user's username using `getpwuid`.
  - Returns "unknown" if the username cannot be retrieved.

- **`char* get_system_name()`**:
  - Retrieves the system's name using `uname`.
  - Returns the node name of the system.

- **`int is_home_directory(const char *cwd)`**:
  - Checks if the current working directory (`cwd`) matches the shell's home directory.
  - Returns `1` if true; otherwise, returns `0`.

- **`void execute_command(char *cmd)`**:
  - Tokenizes the input command string into arguments.
  - Logs the command for tracking.
  - Handles built-in commands such as `hop`, `reveal`, `log`, and `proclore`.
  - Executes external commands using `fork` and `execvp`.
  - Measures execution time for foreground processes and displays it if the process takes more than 2 seconds.

- **`void process_command(char *input)`**:
  - Processes multiple commands separated by semicolons (`;`).
  - Supports background execution using the `&` symbol.
  - Manages the execution of each command, determining if it should run in the foreground or background.

- **`void save_log()`**:
  - Saves the command log to a file named `command_log.txt` in the shell's home directory.
  - Logs are appended to the file, and the total number of commands is limited to 15.

- **`void init_log()`**:
  - Initializes the command log by opening the log file in append mode.
  - Handles errors during initialization by exiting with an error message.

- **`void clear_log()`**:
  - Clears the command log by truncating the log file.
  - Resets the log file to an empty state.

- **`void cleanup_log()`**:
  - Cleans up resources related to the command log.
  - Closes the log file and frees any allocated memory.

- **`void add_to_log(const char *command)`**:
  - Adds a command to the command log.
  - Appends the command to the log file.

- **`void log_command(char **args, int argc)`**:
  - Logs the executed command.
  - Formats the command arguments into a single string for logging.

- **`void hop(char **args, int argc)`****:
  - Implements the `hop` command, which changes the current directory.
  - Supports both absolute and relative paths.
  - Handles errors during directory change and displays appropriate messages.

- **`void seek(char **args, int argc)`**:
  - Implements the `seek` command, which searches for files in a directory.
  - Searches for files based on the provided search term, it decalres a valid match even if the prefix of the file matches the search term.
  - Displays the search results to the user. also supports searching for directories and files based on the provided flags.
  - the flags are -d for directories and -f for files and -e if this flag is effective only when a single file or a single directory with the name is found. If only one file (and no directories) is found, then print it’s output. If only one directory (and no files) is found, then changes current working directory to it. Otherwise, the flag has no effect. This flag should work with -d and -f flags.

- **`void reveal(char **args, int argc)`**: 
  - Implements the `reveal` command, which reveals hidden files or directories based on the provided arguments.
  - It is an almost like an ls command, but it shows the files in a lexico-graphical order and with colour coding for directories and files.
  - Supports revealing hidden files and directories in the current directory by using flags like `-a` and `-l`.
  - Handles errors during the reveal operation.

- **`void proclore(char **args, int argc)`**:
  - Implements the `proclore` command, which displays process information.
  - Displays information about the current shell process and any background processes.
  - Provides details such as process ID, command, and status.

- **'search_directory'**:
  - Searches a specified directory for files matching the search term and populates the results array.
  - Supports searching for directories and files based on the provided flags.

### Command Execution

- The `execute_command` function is responsible for processing and executing user commands.
- It tokenizes the input command string into arguments and logs the command for tracking.
- The function handles built-in commands such as `hop`, `reveal`, `log`, and `proclore`.
- For external commands, the function forks a new process and executes the command using `execvp`.
- Execution time is measured for foreground processes, and a message is displayed if the process takes more than 2 seconds.

### Process Management

- The `process_command` function processes multiple commands separated by semicolons (`;`).
- It supports background execution using the `&` symbol and manages the execution of each command.
- The function determines whether a command should run in the foreground or background based on the input.

### Command Logging

- The shell logs all executed commands to a file named `command_log.txt` in the shell's home directory.
- The log file is created if it does not exist, and commands are appended to the file.
- The total number of commands stored in the log is limited to 15.

### Background Process Management

- Up to `MAX_BG_PROCESSES` (1024) background processes can be tracked.
- The arrays `bg_pids` and `bg_commands` store the PIDs and commands of background processes.
- Background processes are executed by forking a new process and allowing the parent to continue without waiting.

## utils.c

The `utils.c` module provides essential utility functions that support the custom shell's functionality. This includes functions for retrieving user and system information, handling string manipulations, and managing error reporting. The utilities are crucial for command processing and execution within the shell environment.

## Key Functionalities

- **User and System Information**: Functions to retrieve the current user's name and the system's name.
- **String and Path Utilities**: Functions for handling paths and checking directory types.
- **Error Handling**: Centralized error reporting mechanisms for improved maintainability.

## Functions

### `char* get_username()`

- Retrieves the username of the currently logged-in user using the `getpwuid` function.
- Returns `"unknown"` if the username cannot be determined.

### `char* get_system_name()`

- Uses the `uname` function to obtain the system's node name (hostname).
- Returns the hostname as a string.

### `int is_home_directory(const char *cwd)`

- Checks if the provided current working directory (`cwd`) matches the shell's home directory.
- Returns `1` if it matches; otherwise, returns `0`.

### `void handle_error(const char *msg)`

- Centralized error handling function that prints the provided error message to `stderr`.
- Uses `perror` to display the associated system error message based on `errno`.

### `char* get_current_directory()`

- Retrieves the current working directory using `getcwd`.
- Returns a dynamically allocated string containing the directory path.

### `void free_string_array(char **array)`

- Frees a dynamically allocated array of strings.
- Iterates through the array, freeing each string and then the array itself.

# Header Files

## prompt.h

The `prompt.h` header file defines the interface for displaying the command prompt in the shell.

### Functions' brief explanation 

- **`void display_prompt();`**  
  Displays the command prompt, including the current user's username, system name, and the current working directory.

---

## commands.h

The `commands.h` header file declares various command-related functions used in the shell, including built-in commands for logging and directory navigation.

### Constants

- **`#define MAX_PATH 4096`**  
  Maximum path length supported by the shell.

- **`#define MAX_RESULTS 1000`**  
  Maximum number of results returned from search operations.

### Functions

- **`void save_log();`**  
  Saves the command log to persistent storage.

- **`void init_log();`**  
  Initializes the command log.

- **`void clear_log();`**  
  Clears the current command log.

- **`void cleanup_log();`**  
  Cleans up resources related to the command log.

- **`void hop(char **args, int argc);`**  
  Implements the command hop, which changes the current directory.

- **`void seek(char **args, int argc);`**  
  Implements the command seek, which searches for files in a directory.

- **`void reveal(char **args, int argc);`**  
  Reveals hidden files or directories based on the provided arguments.

- **`void proclore(char **args, int argc);`**  
  Implements the command proclore, which displays process information.

- **`void add_to_log(const char *command);`**  
  Adds a command to the log.

- **`void log_command(char **args, int argc);`**  
  Logs the executed command.

- **`void add_colored_path(char *dest, const char *path, const char *color_code);`**  
  Adds a path with a specified color code to the display.

- **`void search_directory(const char *directory, const char *search_term, char results[MAX_RESULTS][MAX_PATH], int *result_count, int d_flag, int f_flag);`**  
  Searches a specified directory for files matching the search term and populates the results array.

---

### Global Variables

- **`extern char *shell_home_directory;`**  
  Global variable that stores the shell's home directory.

## Conclusion

The utilities module and its associated header files provide the necessary declarations for various functionalities implemented in the shell. They enable modular development and ease of maintenance by separating the interface from the implementation.





PROMPTS USED FOR THE PROJECT 

some prompt results used for learning are also saved in the notes.txt file in the repository

https://chatgpt.com/c/59c66f4c-9c85-4e5a-9ac7-75891edc37a1    FOR COMMAND DISCOVERIES
https://chatgpt.com/c/8d64909b-0bf8-45c0-bbe5-91639f1e52e1    FOR FUNCTIONALITY QUESTIONS

