#ifndef COMMANDS_H
#define COMMANDS_H
#define MAX_PATH 4096
#define MAX_RESULTS 1000

void save_log();
void init_log(); 
void clear_log();
void cleanup_log();                
void hop(char **args, int argc);
void seek(char **args, int argc);
void reveal(char **args, int argc); 
void proclore(char **args, int argc);
void add_to_log(const char *command);
void log_command(char **args, int argc); 
void add_colored_path(char *dest, const char *path, const char *color_code);
void search_directory(const char *directory, const char *search_term, char results[MAX_RESULTS][MAX_PATH], int *result_count, int d_flag, int f_flag, int e_flag);

#endif 
