#ifndef ALIAS_H
#define ALIAS_H

#include "globals.h"

void load_myshrc();
void load_functions(const char *myshrc_file) ;
int execute_custom_function(const char *command);
static void add_function(const char *name, const char *body);

#endif