#ifndef COMMANDS_H
#define COMMANDS_H

void hop(char **args, int argc);
void init_log(); 
void cleanup_log();                
void log_command(char **args, int argc); 
void reveal(char **args, int argc); 
void add_to_log(const char *command);
void save_log();
void clear_log();

#endif 
