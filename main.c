#include <stdio.h>
#include <string.h>
#include "treasure_hunt.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Error: Too few arguments!");
    return -1;
  }
  char* op = argv[1];
  if (strcmp(op, "--add") == 0) {
    add_treasure(argv[2]);
  }
  return 0;
}
