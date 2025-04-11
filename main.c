#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure_hunt.h"

int main(int argc, char** argv) {
  if (argc != 3 && argc != 4) {
    printf("Usage: %s <operation> <treasure_hunt_id> [<treasure_id>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char* operation = argv[1];
  char* treasure_hunt_id = argv[2];
  char* treasure_id = (argc == 4) ? argv[3] : NULL;

  if (strcmp(operation, "--add") == 0) {
    add_treasure(treasure_hunt_id);
  } else if (strcmp(operation, "--list") == 0) {
    list_treasure_hunt(treasure_hunt_id);
  } else if (strcmp(operation, "--view") == 0) {
    if (treasure_id == NULL) {
      printf("Treasure ID is required for 'view' operation\n");
      exit(EXIT_FAILURE);
    }
    view_treasure(treasure_hunt_id, treasure_id);
  } else if (strcmp(operation, "--remove_treasure") == 0) {
    if (treasure_id == NULL) {
      printf("Treasure ID is required for 'remove_treasure' operation\n");
      exit(EXIT_FAILURE);
    }
    remove_treasure(treasure_hunt_id, treasure_id);
  } else if (strcmp(operation, "--remove_treasure_hunt") == 0) {
    remove_treasure_hunt(treasure_hunt_id);
  } else {
    printf("Unknown operation '%s'\n", operation);
    exit(EXIT_FAILURE);
  }

  return 0;
}
