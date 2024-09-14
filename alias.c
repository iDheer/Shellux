#include "utils.h"
#include "commands.h"
#include "prompt.h"
#include "alias.h"

void load_myshrc() {
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
                    // printf("Loaded alias: %s -> %s\n", alias_name, command);  // Debug
                    alias_count++;
                }
            }
        }
        fclose(file);
    }
}

bool execute_func_with_argument(const char *func_name, const char *argument){
    char config_path[1024];

    snprintf(config_path, sizeof(config_path), "%s/%s", shell_home_directory, "inesh.myshrc");

    FILE *config_file = fopen(config_path, "r");
    if (!config_file){
        perror("Error opening configuration file");
        return false;
    }

    char buffer[1000];
    bool inside_function = false;
    bool function_found = false;

    while (fgets(buffer, sizeof(buffer), config_file)){
        buffer[strcspn(buffer, "\n")] = '\0'; // Strip newline

        if (!inside_function){
            if (strncmp(func_name, buffer, strlen(func_name)) == 0 &&
                strstr(buffer, "()") != NULL && buffer[strlen(func_name)] == '('){
                inside_function = true;
                function_found = true;
                continue;
            }
        }
        else{
            if (strchr(buffer, '{')){
                continue;
            }
            if (strchr(buffer, '}')){
                break;
            }

            char *arg_position = strstr(buffer, "$1");
            if (arg_position){
                char modified_command[1000];
                size_t prefix_len = arg_position - buffer;

                snprintf(modified_command, sizeof(modified_command), "%.*s%s%s", (int)prefix_len, buffer, argument, arg_position + 2);

                execute_command(modified_command, 0);
            }
            else{
                execute_command(buffer, 0);
            }
        }
    }

    fclose(config_file);
    return function_found;
}

