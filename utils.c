#include "utils.h"
#include "commands.h"
#include "prompt.h"

// Initialize the shell home directory
void initialize_shell_home_directory() {
    shell_home_directory = getcwd(NULL, 0); // Retrieve current working directory
    if (!shell_home_directory) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}

// Get current username
char* get_username() {
    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_name : "unknown"; // Handle case where username can't be retrieved
}

// Get system name
char* get_system_name() {
    static struct utsname unameData;
    uname(&unameData);
    return unameData.nodename;
}

// Check if the current directory is the home directory
int is_home_directory(const char *cwd) {
    return strcmp(cwd, shell_home_directory) == 0;
}

// Purge the oldest background commands from the global array
void purge_oldest_background_commands() {
    int purge_count = MAX_BG_PROCESSES / 2; // Purge the oldest 50 commands

    // Shift remaining commands up by 50
    for (int i = purge_count; i < bg_count; i++) {
        bg_processes[i - purge_count] = bg_processes[i];
    }

    // Reduce the background process count
    bg_count -= purge_count;
}

// Execute a command with redirection and background handling
void execute_command(char *cmd, int is_background) {
    char *args[4096];
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;
    int argc = 0;

    // Parse the command string and handle redirection
    while (*cmd != '\0') {
        while (*cmd == ' ' || *cmd == '\t' || *cmd == '\n') {
            cmd++;
        }
        if (*cmd == '\0') break;

        // Handle input redirection
        if (*cmd == '<') {
            cmd++;
            while (*cmd == ' ' || *cmd == '\t') cmd++; // Skip spaces
            input_file = cmd;
            while (*cmd != ' ' && *cmd != '\t' && *cmd != '\n' && *cmd != '\0') cmd++;
            if (*cmd != '\0') {
                *cmd = '\0';
                cmd++;
            }
        }
        // Handle output redirection
        else if (*cmd == '>') {
            cmd++;
            if (*cmd == '>') {  // Check for append mode (>>)
                append_mode = 1;
                cmd++;
            }
            while (*cmd == ' ' || *cmd == '\t') cmd++; // Skip spaces
            output_file = cmd;
            while (*cmd != ' ' && *cmd != '\t' && *cmd != '\n' && *cmd != '\0') cmd++;
            if (*cmd != '\0') {
                *cmd = '\0';
                cmd++;
            }
        }
        // Handle regular arguments
        else {
            args[argc++] = cmd;
            while (*cmd != ' ' && *cmd != '\t' && *cmd != '\n' && *cmd != '\0') cmd++;
            if (*cmd != '\0') {
                *cmd = '\0';
                cmd++;
            }
        }
    }

    args[argc] = NULL;  // Null-terminate the arguments array

    if (args[0] == NULL) return;  // No command entered

    // Log the command
    char log_entry[4096];
    snprintf(log_entry, sizeof(log_entry), "%s", args[0]);
    for (int i = 1; i < argc; i++) {
        snprintf(log_entry + strlen(log_entry), sizeof(log_entry) - strlen(log_entry), " %s", args[i]);
    }

    // Handle built-in commands
    if (strcmp(args[0], "activities") == 0) {
        activities();  // Call the activities function
        return;  // Skip further processing
    } else if (strcmp(args[0], "hop") == 0) {
        hop(args, argc);
        return;
    } else if (strcmp(args[0], "reveal") == 0) {
        reveal(args, argc);
        return;
    } else if (strcmp(args[0], "log") == 0) {
        log_command(args, argc);
        return;
    } else if (strcmp(args[0], "proclore") == 0) {
        proclore(args, argc);
        return;
    } else if (strcmp(args[0], "seek") == 0) {
        seek(args, argc);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {  // Child process
        // Handle input redirection
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("No such input file found!");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);  // Redirect stdin to the input file
            close(fd_in);
        }

        // Handle output redirection
        if (output_file != NULL) {
            int fd_out;
            if (append_mode) {
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd_out < 0) {
                perror("Failed to open output file!");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);  // Redirect stdout to the output file
            close(fd_out);
        }

        // Execute the command
        if (execvp(args[0], args) == -1) {
            perror("execvp");  // Execution failed
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) { // Parent process
        if (is_background) {
            // Handle background process
            if (bg_count >= MAX_BG_PROCESSES) {
                purge_oldest_background_commands();
            }
            bg_processes[bg_count].pid = pid; // Store the PID of the background process
            bg_processes[bg_count].command = strdup(log_entry); // Store the command (log_entry contains the full command)
            if (!bg_processes[bg_count].command) {
                perror("strdup");
                exit(EXIT_FAILURE);
            }
            strcpy(bg_processes[bg_count].state, "Running"); // Initially mark as running
            bg_count++;
            printf("Background PID: %d\n", pid);
        } else {
            // Foreground process: measure execution time
            struct timeval start, end;
            gettimeofday(&start, NULL); // Record start time

            int status;
            waitpid(pid, &status, 0); // Wait for the child process to finish

            gettimeofday(&end, NULL); // Record end time

            long seconds = end.tv_sec - start.tv_sec;

            if (seconds > 2) {
                printf("<%s@%s:~ %s : %ld seconds>\n", get_username(), get_system_name(), log_entry, seconds);
            }

            // For foreground commands, mark the command as terminated
            if (bg_count >= MAX_BG_PROCESSES) {
                purge_oldest_background_commands();
            }
            bg_processes[bg_count].pid = pid;
            bg_processes[bg_count].command = strdup(log_entry);
            strcpy(bg_processes[bg_count].state, "Terminated");
            bg_count++;
        }
    } else {
        perror("fork");
    }
}

// Tokenize the command string
void tokenize_command(char *command, char **args) {
    char *ptr = command;
    int arg_count = 0;
    char *token;
    char *end_ptr;

    while (*ptr) {
        // Skip whitespace
        while (*ptr == ' ') {
            ptr++;
        }

        // Handle quoted strings
        if (*ptr == '\'' || *ptr == '\"') {
            char quote = *ptr++;
            token = ptr; // Start of the token
            while (*ptr && *ptr != quote) {
                ptr++;
            }
            if (*ptr == quote) {
                *ptr++ = '\0'; // Null-terminate the token
                args[arg_count++] = token; // Save the argument
            }
            continue;
        }

        // Handle normal tokens
        end_ptr = ptr;
        while (*end_ptr && *end_ptr != ' ') {
            end_ptr++;
        }

        if (end_ptr != ptr) {
            *end_ptr = '\0'; // Null-terminate the token
            args[arg_count++] = ptr; // Save the argument
            ptr = end_ptr + 1; // Move past the token
        }
    }
    args[arg_count] = NULL; // Null-terminate the arguments array
}

void execute_piped_commands(char *piped_commands[], int count, int is_background) {
    int pipefds[2 * (count - 1)];
    pid_t pids[count];
    char *input_file = NULL;
    char *output_file = NULL;
    int append_mode = 0;

    // Parse redirection from the first and last command
    if (strchr(piped_commands[0], '<')) {
        char *first_cmd = piped_commands[0];
        input_file = strtok(first_cmd, "<");
        input_file = strtok(NULL, "<");
        input_file = trim_whitespace(input_file);
        piped_commands[0] = first_cmd;  // Update the command without redirection part
    }

    if (strchr(piped_commands[count - 1], '>')) {
        char *last_cmd = piped_commands[count - 1];
        if (strstr(last_cmd, ">>")) {
            append_mode = 1;
            output_file = strtok(last_cmd, ">>");
            output_file = strtok(NULL, ">>");
        } else {
            output_file = strtok(last_cmd, ">");
            output_file = strtok(NULL, ">");
        }
        output_file = trim_whitespace(output_file);
        piped_commands[count - 1] = last_cmd;  // Update the command without redirection part
    }

    // Create pipes
    for (int i = 0; i < count - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < count; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return;
        }

        if (pids[i] == 0) { // Child process
            // Handle input redirection for the first command
            if (i == 0 && input_file != NULL) {
                int fd_in = open(input_file, O_RDONLY);
                if (fd_in < 0) {
                    perror("Failed to open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            // Handle output redirection for the last command
            if (i == count - 1 && output_file != NULL) {
                int fd_out;
                if (append_mode) {
                    fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else {
                    fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if (fd_out < 0) {
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_out, STDOUT_FILENO);
                close(fd_out);
            }

            // Set up pipe input/output
            if (i != 0) {  // Not the first command
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO); // Input from previous pipe
            }
            if (i != count - 1) {  // Not the last command
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO); // Output to next pipe
            }

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (count - 1); j++) {
                close(pipefds[j]);
            }

            // Tokenize and execute command
            char *args[4096];
            tokenize_command(piped_commands[i], args);
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Close pipe file descriptors in parent
    for (int i = 0; i < 2 * (count - 1); i++) {
        close(pipefds[i]);
    }

    // Parent waits for children
    if (!is_background) {
        for (int i = 0; i < count; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        for (int i = 0; i < count; i++) {
            printf("Background process PID: %d\n", pids[i]);
        }
    }
}

// Function to trim whitespace from the beginning and end of a string
char *trim_whitespace(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0'; // Null-terminate after the last non-space character

    return str;
}

void process_command(char *input) {
    // Trim whitespace from the input command
    char *trimmed_command = trim_whitespace(input);
    if (strlen(trimmed_command) == 0) {
        return; // Skip empty commands
    }

    // Split the command into separate commands based on ';'
    char *commands[100]; // Adjust size as needed
    int command_count = 0;

    char *command = strtok(trimmed_command, ";");
    while (command != NULL) {
        commands[command_count++] = command;
        command = strtok(NULL, ";");
    }

    // Iterate over each command
    for (int i = 0; i < command_count; i++) {
        char *final_command = commands[i];
        int is_background = 0;

        // Trim whitespace from the command
        final_command = trim_whitespace(final_command);

        // Check for the activities command
        if (strcmp(final_command, "activities") == 0) {
            activities(); // Call the activities function to display process list
            continue; // Skip executing further commands
        }

        // Search for '&' within the command
        char *ampersand_position = strchr(final_command, '&');
        if (ampersand_position != NULL) {
            is_background = 1;
            *ampersand_position = '\0'; // Split the command at '&'
            final_command = trim_whitespace(final_command); // Trim again to clean up after '&'
            // printf("Background process detected: %s\n", final_command);
        }

        // Split the command into piped commands
        char *piped_commands[100]; // Adjust size as needed
        int piped_command_count = 0;

        char *pipe_cmd = strtok(final_command, "|");
        while (pipe_cmd != NULL) {
            piped_commands[piped_command_count++] = pipe_cmd;
            pipe_cmd = strtok(NULL, "|");
        }

        // Execute based on the number of piped commands
        if (piped_command_count == 1) {
            // Single command
            execute_command(piped_commands[0], is_background);
        } else {
            // Multiple piped commands
            execute_piped_commands(piped_commands, piped_command_count, is_background);
        }

        // If there's another command after '&' (which wasn't split by ';'), execute it in the foreground
        if (ampersand_position != NULL && *(ampersand_position + 1) != '\0') {
            char *remaining_command = trim_whitespace(ampersand_position + 1);
            if (strlen(remaining_command) > 0) {
                // printf("Foreground part after '&': %s\n", remaining_command);
                process_command(remaining_command); // Process the remaining command
            }
        }
    }
}

void activities() {
    // Update process statuses
    for (int i = 0; i < bg_count; i++) {
        int status;
        pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG);
        
        // If the process is still running
        if (result == 0) {
            strcpy(bg_processes[i].state, "Running"); // Update state to Running
        } else if (result > 0) {
            strcpy(bg_processes[i].state, "Stopped"); // Update state to Stopped
        }
    }

    // Print out the processes
    for (int i = 0; i < bg_count; i++) {
        printf("[%d] : %s - %s\n", bg_processes[i].pid, bg_processes[i].command, bg_processes[i].state);
    }
}