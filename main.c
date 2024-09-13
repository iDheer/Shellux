#include "utils.h"
#include "commands.h"
#include "prompt.h"

int bg_count = 0;
int alias_count = 0;
pid_t foreground_pid = -1; 
char* shell_home_directory;
Alias aliases[MAX_ALIASES];
static Function *function_list = NULL;
ProcessInfo bg_processes[MAX_BG_PROCESSES];

void handle_error(const char *message) {
    fprintf(stderr, COLOR_RED "Error: %s: %s\n" COLOR_RESET, message, strerror(errno));
}

extern void cleanup_bg_processes() {
    for (int i = 0; i < bg_count; i++) {
        free(bg_processes[i].command); 
        bg_processes[i].command = NULL; // Avoid dangling pointers
    }
    bg_count = 0; 
}

extern void remove_background_process(pid_t pid) {
    for (int i = 0; i < bg_count; i++) {
        if (bg_processes[i].pid == pid) {
            free(bg_processes[i].command); 
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_count--;
            // Only set the last command to NULL if there are remaining processes
            if (bg_count > 0) {
                bg_processes[bg_count].command = NULL; // Set last command to NULL
            }
            break;
        }
    }
}

extern void add_to_background_processes(pid_t pid, char *command) {
    if (bg_count < MAX_BG_PROCESSES) {
        bg_processes[bg_count].pid = pid;
        bg_processes[bg_count].command = strdup(command);
        if (bg_processes[bg_count].command == NULL) {
            handle_error("Failed to allocate memory for background process command");
            exit(EXIT_FAILURE);
        }
        // strcpy(bg_processes[bg_count].state, state);
        bg_count++;
    } else {
        purge_oldest_background_commands(); // Remove the oldest background process
        add_to_background_processes(pid, command); // Try again
    }
}

void custom_handler(int signum) {
    if (signum == SIGINT) { // Ctrl + C
        if (foreground_pid > 0) {
            kill(foreground_pid, SIGINT); // Terminate foreground process
            printf("Terminated foreground process (PID: %d)\n", foreground_pid);
            update_foreground_pid(-1); // Reset foreground PID
        } else {
            printf("\nNo foreground process to terminate\n");
        }
    } 
    else if (signum == SIGTSTP) { // Ctrl + Z
        if (foreground_pid > 0) {
            kill(foreground_pid, SIGTSTP); // Stop foreground process
            printf("Stopped foreground process (PID: %d)\n", foreground_pid);
            add_to_background_processes(foreground_pid, get_command_name(foreground_pid));
            update_foreground_pid(-1); // Reset foreground PID
        } else {
            printf("\nNo foreground process to stop\n");
        }
    }
}

// Trim leading and trailing whitespace for alias processing
char* trim_whitespace_a(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    end[1] = '\0';

    return str;
}

extern void load_myshrc() {
    // Process aliases from file (if needed)
    FILE *file = fopen("inesh.myshrc", "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            // Process aliases
            if (strstr(line, "alias") != NULL) {
                char *alias_name = strtok(line + 6, "=");
                char *command = strtok(NULL, "\n");

                alias_name = trim_whitespace_a(alias_name);
                command = trim_whitespace_a(command);

                if (alias_name && command && alias_count < MAX_ALIASES) {
                    aliases[alias_count].alias_name = strdup(alias_name);
                    aliases[alias_count].command = strdup(command);
                    printf("Loaded alias: %s -> %s\n", alias_name, command);  // Debug
                    alias_count++;
                }
            }
        }
        fclose(file);
    }
}
// Function to add a new function to the list
static void add_function(const char *name, const char *body) {
    Function *new_func = malloc(sizeof(Function));
    if (new_func == NULL) {
        perror("Error allocating memory for function");
        exit(EXIT_FAILURE);
    }

    strncpy(new_func->name, name, MAX_FUNCTION_NAME - 1);
    new_func->name[MAX_FUNCTION_NAME - 1] = '\0';
    strncpy(new_func->body, body, MAX_FUNCTION_BODY - 1);
    new_func->body[MAX_FUNCTION_BODY - 1] = '\0';
    new_func->next = function_list;
    function_list = new_func;
}

// Load functions from the .myshrc file
void load_functions(const char *myshrc_file) {
    // printf("%d\n", 1111);  // Debug statement
    FILE *file = fopen(myshrc_file, "r");
    if (file == NULL) {
        perror("Error opening .myshrc file");
        return;
    }

    char line[1024];
    char function_name[MAX_FUNCTION_NAME];
    char function_body[MAX_FUNCTION_BODY];
    int in_function = 0;

    while (fgets(line, sizeof(line), file)) {
        // Strip comments (ignore anything after #)
        char *comment_pos = strchr(line, '#');
        if (comment_pos != NULL) {
            *comment_pos = '\0';  // Null-terminate to ignore the comment
        }

        // Trim leading/trailing whitespace
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;  // Trim leading spaces
        char *end = trimmed_line + strlen(trimmed_line) - 1;
        while (end > trimmed_line && isspace(*end)) end--;  // Trim trailing spaces
        *(end + 1) = '\0';  // Null-terminate after trimming

        // Check for function start
        if (strncmp(trimmed_line, "func ", 5) == 0) {
            in_function = 1;
            sscanf(trimmed_line + 5, "%s", function_name);  // Get the function name

            // Remove parentheses if they exist
            char *paren_pos = strchr(function_name, '(');
            if (paren_pos) {
                *paren_pos = '\0';  // Null-terminate to remove the parentheses
            }

            function_body[0] = '\0';  // Reset function body
        } 
        else if (in_function && strstr(trimmed_line, "{") != NULL) {
            // Ignore the opening curly brace '{'
            continue;
        }
        else if (in_function && strncmp(trimmed_line, "}", 1) == 0) {
            // Function ends at closing curly brace
            in_function = 0;
            add_function(function_name, function_body);
            printf("Loaded function: %s\n", function_name);  // Debug
        } 
        else if (in_function && strlen(trimmed_line) > 0) {
            // Append lines to function body if it's not empty
            strncat(function_body, trimmed_line, sizeof(function_body) - strlen(function_body) - 1);
            strncat(function_body, "\n", sizeof(function_body) - strlen(function_body) - 1);  // Add newline
        }
    }

    fclose(file);
}

// Execute a custom function if it matches
int execute_custom_function(const char *command) {
    char function_name[MAX_FUNCTION_NAME];
    char function_args[MAX_FUNCTION_BODY] = {0};
    char *arg_start;

    // Extract function name
    sscanf(command, "%s", function_name);

    // Extract arguments if present
    arg_start = strchr(command, ' ');
    if (arg_start) {
        arg_start++;  // Move past the space
        strcpy(function_args, arg_start);
    }

    // Search for the function in the function list
    Function *func = function_list;
    while (func != NULL) {
        if (strcmp(func->name, function_name) == 0) {
            // Duplicate function body
            char *body_copy = strdup(func->body);
            if (!body_copy) {
                perror("Error duplicating function body");
                return 0;
            }

            // Replace both $1 and "$1" with function_args (without adding quotes)
            char *new_body = malloc(MAX_FUNCTION_BODY);
            if (!new_body) {
                perror("Error allocating memory for function body");
                free(body_copy);
                return 0;
            }
            new_body[0] = '\0';  // Start with an empty string

            char *current_pos = body_copy;
            while (*current_pos) {
                // Check for $1
                char *arg_pos = strstr(current_pos, "$1");
                char *quoted_arg_pos = strstr(current_pos, "\"$1\"");
                
                if (arg_pos && (!quoted_arg_pos || arg_pos < quoted_arg_pos)) {
                    // Handle $1 without quotes
                    strncat(new_body, current_pos, arg_pos - current_pos);  // Copy part before $1
                    strcat(new_body, function_args);  // Insert function_args
                    current_pos = arg_pos + 2;  // Move past $1
                } else if (quoted_arg_pos) {
                    // Handle "$1"
                    strncat(new_body, current_pos, quoted_arg_pos - current_pos);  // Copy part before "$1"
                    strcat(new_body, function_args);  // Insert function_args
                    current_pos = quoted_arg_pos + 4;  // Move past "$1"
                } else {
                    // No more $1 or "$1", copy the rest
                    strcat(new_body, current_pos);
                    break;
                }
            }


        char* new_segment;
        char* new_segments;
        new_segment = strtok_r(new_body, "\n",&new_segments);
        while(new_segment != NULL){
            process_command(new_segment);
            new_segment = strtok_r(NULL, "\n",&new_segments);
        }

            free(body_copy);
            free(new_body);
            return 1;  // Exit after executing the matched function
        }
        func = func->next;
    }
    return 0;
}


int main() {
    char command[4096];
    init_log(); 
    load_myshrc();
    load_functions("inesh.myshrc");
    initialize_shell_home_directory(); 

    // Set signal handlers for SIGINT and SIGTSTP
    struct sigaction sa;
    sa.sa_handler = custom_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        handle_error("Failed to set SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        handle_error("Failed to set SIGTSTP handler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        display_prompt();  
        if (fgets(command, sizeof(command), stdin) != NULL) {
            command[strcspn(command, "\n")] = 0; // Remove trailing newline characters

            if (strcmp(command, "exit") == 0) { // Self-defined exit command
                break;
            }
            add_to_log(command); // Enter the command in the log
            process_command(command); // Main shell logic
        } else {
            if (feof(stdin)) {
                // Log out and kill all background processes
                for (int i = 0; i < bg_count; i++) {
                    if (kill(bg_processes[i].pid, SIGTERM) == -1) {
                        handle_error("Failed to terminate background process");
                    }
                }
                break; // Exit the shell
            } else {
                continue; // Ignore empty input
            }
        }
    }

    cleanup_bg_processes(); // Clean up allocated memory for background processes
    cleanup_log(); // Save the log at shutdown
    return 0;
}
