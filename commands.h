#ifndef COMMANDS_H
#define COMMANDS_H

void save_log();
void init_log(); 
void clear_log();
void cleanup_log();                
void hop(char **args, int argc);
void reveal(char **args, int argc); 
void proclore(char **args, int argc);
void add_to_log(const char *command);
void log_command(char **args, int argc); 

#endif 
