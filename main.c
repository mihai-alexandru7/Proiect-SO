#include <stdio.h>
#include <string.h>
#include "treasure_hunt.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("Usage: %s <operation> <treasure_hunt_id> [<treasure_id>]\n", argv[0]);
    return -1;
  }

  char* operation = argv[1];
  char* treasure_hunt_id = argv[2];
  char* treasure_id = (argc == 4) ? argv[3] : NULL;

  if (strcmp(operation, "--add") == 0) {
    add_treasure(treasure_hunt_id);
  } else if (strcmp(operation, "--list") == 0) {
    list_treasure_hunt(treasure_hunt_id);
  } else if (strcmp(operation, "--view") == 0) {
    view_treasure(treasure_hunt_id, treasure_id);
  } else {
    printf("Unknown operation '%s'\n", operation);
  }

  return 0;
}
