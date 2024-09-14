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
                    printf("Loaded alias: %s -> %s\n", alias_name, command);  // Debug
                    alias_count++;
                }
            }
        }
        fclose(file);
    }
}

bool execute_function_with_arg(const char *function_name, const char *arg)
{
    char CONFIGPATH[1024];


    strcpy(CONFIGPATH, shell_home_directory);
    strcat(CONFIGPATH, "/inesh.myshrc");

    FILE *file = fopen(CONFIGPATH, "r");
    if (file == NULL)
    {
        perror("Failed to open .myshrc");
        return false;
    }

    char line[1000];
    int in_function = 0;
    bool b = false;
    while (fgets(line, 1000, file))
    {
        line[strcspn(line, "\n")] = 0;
        if (strncmp(function_name, line, strlen(function_name)) == 0 &&
            strstr(line, "()") != NULL && line[strlen(function_name)] == '(')
        {
            in_function = 1;
            b = true;
            continue;
        }
        if (in_function)
        {
            if (strstr(line, "{"))
            {
                continue;
            }

            if (strchr(line, '}'))
            {
                break;
            }
 
            char *placeholder = strstr(line, "$1");
            if (placeholder != NULL) {
                char modified_line[1000];
                int prefix_length = placeholder - line; // Length of the part before $1

                snprintf(modified_line, sizeof(modified_line), "%.*s%s%s", 
                        prefix_length, line, arg, placeholder + 2);

                printf("Executing: %s\n", modified_line);
                execute_command(modified_line, 0);
            }
            else
            {
                execute_command(line, 0);
            }
        }
    }
    fclose(file);
    return b;
}
