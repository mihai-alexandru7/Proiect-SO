#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 5

char** get_argv(const char* cmd, int* argc);
void free_argv(char** argv, int argc);

#endif