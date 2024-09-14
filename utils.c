#include "utils.h"
#include "commands.h"
#include "prompt.h"
#include "alias.h"

void handle_sigchld(int sig);

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
        // printf("args[1] before the final null terminator: %s\n", args[1]);
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


    if(execute_function_with_arg(args[0], args[1])==true){
        return;
    }

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
    } else if (strcmp(args[0], "ping") == 0) {
        // Handle ping command
        if (argc != 3) {
            fprintf(stderr, "Usage: ping <pid> <signal_number>\n");
            return;
        }

        pid_t pid = atoi(args[1]); // Convert string to PID
        int signal_number = atoi(args[2]); // Convert string to signal number

        ping_process(pid, signal_number);
        return; // Exit this function after handling the ping command
    } else if (strcmp(args[0], "fg") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Usage: fg <pid>\n");
            return;
        }
        pid_t pid = atoi(args[1]);
        fg_process(pid);
        return;
    } else if (strcmp(args[0], "bg") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Usage: bg <pid>\n");
            return;
        }
        pid_t pid = atoi(args[1]);
        bg_process(pid);
        return;
    } else if (strcmp(args[0], "iMan") == 0) {
        if (argc != 2) {
            fprintf(stderr, "Usage: iman <command_name>\n");
            return;
        }
        iMan(args[1]);  // iMan function with the command
        return;
    } else if (strcmp(args[0], "neonate") == 0) {
        if (argc < 2) {
            fprintf(stderr, "Usage: neonate [-n time_arg]\n");
            return;
        }
        
        // Check if the flag is present
        if (strcmp(args[1], "-n") == 0) {
            if (argc != 3) {
                fprintf(stderr, "Usage: neonate -n [time_arg]\n");
                return;
            }
            
            int time_arg = atoi(args[2]);
            if (time_arg <= 0) {
                fprintf(stderr, "Error: time_arg must be a positive integer.\n");
                return;
            }
            
            char command[100];
            snprintf(command, sizeof(command), "neonate -n %d", time_arg);
            handleNeonateCommand(command);
            return;
        } else {
            // If no flag is provided, just display the last PID
            handleNeonateCommand(args[0]);
            return;
        }
    }



    //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    pid_t pid = fork();
    //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    if (pid < 0) {
        handle_error("Error forking process");
        return; // Return to avoid continuing with invalid pid
    } 
    else if (pid == 0) {  // Child process
        // Reset signal handlers to default for the child process
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // Set the process group based on whether it's a background process
        if (is_background) {
            setpgid(0, 0);  // Create a new process group for the background process
        } 

        // Input redirection
        if (input_file != NULL) {
            int fd_in = open(input_file, O_RDONLY);

            if (fd_in < 0) {
                handle_error("Failed to open input file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);  
            close(fd_in);
        }

        // Output redirection
        if (output_file != NULL) {
            int fd_out = append_mode ? open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644) 
                                    : open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                handle_error("Failed to open output file");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);  
            close(fd_out);
        }

        // Check for aliases only for non built-in commands
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(args[0], aliases[i].alias_name) == 0) {
                // Parse the aliased command and execute it
                char *cmdcmd= strdup(aliases[i].command);
                char *token = strtok(cmdcmd, " ");
                char *exec_args[100]; // Adjust size as needed
                int arg_index = 0;
                while (token != NULL && arg_index < 99) {  // Adjusted index limit
                    exec_args[arg_index++] = token;
                    token = strtok(NULL, " ");
                }
                exec_args[arg_index] = NULL;  // Null-terminate the arguments
                execvp(exec_args[0], exec_args); // Execute the command
                free(cmdcmd);
                return;
            }
        }

        if (execvp(args[0], args) == -1) {
                    // printf("reached");
            handle_error("Command execution failed");
            exit(EXIT_FAILURE);
        }
    }
    
    else {  // Parent process
        if (!is_background) {
            update_foreground_pid(pid);
            
            // Measure execution time
            struct timeval start, end;
            gettimeofday(&start, NULL);  // Start time
            
            int status;
            do {
                if (waitpid(pid, &status, 0) == -1) {
                    if (errno == EINTR) {
                        continue;  // Interrupted by a signal, try again
                    } else {
                        handle_error("Error waiting for child process");
                        break;
                    }
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));  // Keep waiting until process exits or is killed

            
            gettimeofday(&end, NULL);  // End time
            long seconds = end.tv_sec - start.tv_sec;
            
            if (seconds > 2) { 
                printf("<%s@%s:~ %s : %ld seconds>\n", get_username(), get_system_name(), log_entry, seconds);
            }
            
            // Reset foreground PID after command execution
            update_foreground_pid(-1);
        } 
        else {
            // Background process handling
            setpgid(pid, pid);  // Set the child process group ID
            add_to_background_processes(pid, log_entry);
            printf("Background PID: %d\n", pid);
        }
    }
}

// Tokenize the command string
void tokenize_command(char *command, char **args, int *is_background) {
    char *ptr = command;
    int arg_count = 0;
    char *token;
    char *end_ptr;

    // Check for background execution symbol
    if (command[strlen(command) - 1] == '&') {
        *is_background = 1;  // Set background flag
        command[strlen(command) - 1] = '\0';  // Remove '&' from command
    } else {
        *is_background = 0;
    }

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
        char *first_cmd = strtok(piped_commands[0], "<");
        input_file = strtok(NULL, "<");
        input_file = trim_whitespace(input_file);
        piped_commands[0] = trim_whitespace(first_cmd);  // Update the command without redirection part
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
        piped_commands[count - 1] = trim_whitespace(last_cmd);  // Update the command without redirection part
    }

    // Create pipes
    for (int i = 0; i < count - 1; i++) {
        if (pipe(pipefds + i * 2) == -1) {
            handle_error("Failed to create pipe");
            return;
        }
    }

    for (int i = 0; i < count; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            handle_error("Failed to fork process");
            return;
        }

        if (pids[i] == 0) { // Child process
            // Set the process group ID to the PID of the first child
            if (i == 0) {
                setpgid(0, 0);
            }

            // Handle input redirection for the first command
            if (i == 0 && input_file != NULL) {
                int fd_in = open(input_file, O_RDONLY);
                if (fd_in < 0) {
                    handle_error("Failed to open input file");
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
                    handle_error("Failed to open output file");
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
            int background_flag = 0; // Flag to check for background execution
            tokenize_command(piped_commands[i], args, &background_flag); // Updated call
            if (execvp(args[0], args) == -1) {
                handle_error("Command execution failed");
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

        // Check for '&' to determine background process
        char *ampersand_position = strchr(final_command, '&');
        if (ampersand_position != NULL) {
            is_background = 1;
            *ampersand_position = '\0'; // Split the command at '&'
            final_command = trim_whitespace(final_command); // Trim again to clean up after '&'
        }

        // Split the command into piped commands
        char *piped_commands[100]; // Adjust size as needed
        int piped_command_count = 0;

        char *pipe_cmd = strtok(final_command, "|");
        while (pipe_cmd != NULL) {
            piped_commands[piped_command_count++] = trim_whitespace(pipe_cmd); // Trim each piped command
            pipe_cmd = strtok(NULL, "|");
        }

        // Execute based on the number of piped commands
        if (piped_command_count == 1) {
            // Single command

            // char func[1000], func_arg[1000];
            // char *p = strstr(piped_commands[0], " ");
            // if (p == NULL)
            // {

            //     strcpy(func, piped_commands[0]);

            //     bool b = execute_function_with_arg(func, func_arg);
            //     if (b)
            //     {
            //         return;
            //     }
            // }
            // else
            // {
            //     int i = 0;
            //     while (piped_commands[0][i] != ' ')
            //     {
            //         func[i] = piped_commands[0][i];
            //         i++;
            //     }
            //     func[i] = '\0';

            //     bool b = execute_function_with_arg(func, p + 1);
            //     if (b)
            //     {
            //         return;
            //     }
            // }

            execute_command(piped_commands[0], is_background);
        } else {
            // Multiple piped commands
            execute_piped_commands(piped_commands, piped_command_count, is_background);
        }

        // If there's another command after '&' (which wasn't split by ';'), process it
        if (is_background && *(ampersand_position + 1) != '\0') {
            char *remaining_command = trim_whitespace(ampersand_position + 1);
            if (strlen(remaining_command) > 0) {
                process_command(remaining_command); // Process the remaining command
            }
        }
    }
}

void activities() {
    for (int i = 0; i < bg_count; i++) {
        char proc_status_path[256];
        char line[256];
        snprintf(proc_status_path, sizeof(proc_status_path), "/proc/%d/status", bg_processes[i].pid);

        FILE *status_file = fopen(proc_status_path, "r");
        if (status_file == NULL) {
            // Process likely terminated
            strcpy(bg_processes[i].state, "Terminated");
            continue;
        }

        // Check process status
        while (fgets(line, sizeof(line), status_file)) {
            if (strncmp(line, "State:", 6) == 0) {
                char proc_state;
                sscanf(line, "State: %c", &proc_state);

                if (proc_state == 'T') {
                    strcpy(bg_processes[i].state, "Stopped");
                } else if (proc_state == 'Z') {
                    strcpy(bg_processes[i].state, "Terminated");  // Zombie process
                } else {
                    strcpy(bg_processes[i].state, "Running");
                }
                break;
            }
        }

        fclose(status_file);
    }

    // Print out the processes
    for (int i = 0; i < bg_count; i++) {
        printf("[%d] : %s - %s\n", bg_processes[i].pid, bg_processes[i].command, bg_processes[i].state);
    }

    // Cleanup background processes that are "Terminated"
    for (int i = 0; i < bg_count; i++) {
        if (strcmp(bg_processes[i].state, "Terminated") == 0) {
            remove_background_process(i);  // Remove terminated process from the list
            i--;  // Adjust index since the array has shifted
        }
    }
}

