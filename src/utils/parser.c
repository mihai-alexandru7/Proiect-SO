#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"

char** get_argv(const char* cmd, int* argc) {
  if (!cmd) {
    return NULL;
  }
  char* cmd_copy = strdup(cmd);
  if (!cmd_copy) {
    return NULL;
  }
  char** argv = (char**)malloc(MAX_ARGS * sizeof(char*));
  if (!argv) {
    free(cmd_copy);
    return NULL;
  }
  *argc = 0;
  char* arg = strtok(cmd_copy, " ");
  while (arg && *argc < MAX_ARGS) {
    argv[(*argc)++] = strdup(arg);
    arg = strtok(NULL, " ");
  }
  argv[*argc] = NULL;
  free(cmd_copy);
  return argv;
}

void free_argv(char** argv, int argc) {
  if(!argv) {
    return;
  }
  for(int i = 0; i < argc; i++) {
    free(argv[i]);
  }
  free(argv);
}