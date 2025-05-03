#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treasure.h"
#include "treasure_operations.h"

// Print usage instructions
void print_usage(const char* program_name) {
  printf("Usage:\n");
  printf("'%s --add <hunt_id>' to add a new treasure (prompts for details)\n", program_name);
  printf("'%s --view_treasure <hunt_id> <treasure_id>' to view details of a specific treasure\n", program_name);
  printf("'%s --list_treasures <hunt_id>' to list all treasures in a hunt\n", program_name);
  printf("'%s --list_hunts' to list all available hunts\n", program_name);
  printf("'%s --remove_treasure <hunt_id> <treasure_id>' to remove a specific treasure\n", program_name);
  printf("'%s --remove_hunt <hunt_id>' to remove an entire hunt\n", program_name);
}

int main(int argc, char** argv) {
  if (argc < 2 || argc > 4) {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  // Process operations
  char* operation = argv[1];
  char* hunt_id = (argc > 2) ? argv[2] : NULL;
  char* treasure_id = (argc == 4) ? argv[3] : NULL;
  if (strcmp(operation, "--add_treasure") == 0) {
    if (hunt_id == NULL) {
      printf("Hunt ID is required for '%s' operation\n", operation);
      exit(EXIT_FAILURE);
    }
    add_treasure(hunt_id);
  } else if (strcmp(operation, "--view_treasure") == 0) {
    if (hunt_id == NULL || treasure_id == NULL) {
      printf("Hunt ID and Treasure ID is required for '%s' operation\n", operation);
      exit(EXIT_FAILURE);
    }
    view_treasure(hunt_id, treasure_id);
  } else if (strcmp(operation, "--list_treasures") == 0) {
    if (hunt_id == NULL) {
      printf("Hunt ID is required for '%s' operation\n", operation);
      exit(EXIT_FAILURE);
    }
    list_treasures(hunt_id);
  } else if (strcmp(operation, "--list_hunts") == 0) {
    list_hunts();
  } else if (strcmp(operation, "--remove_treasure") == 0) {
    if (hunt_id == NULL || treasure_id == NULL) {
      printf("Hunt ID and Treasure ID is required for '%s' operation\n", operation);
      exit(EXIT_FAILURE);
    }
    remove_treasure(hunt_id, treasure_id);
  } else if (strcmp(operation, "--remove_hunt") == 0) {
    if (hunt_id == NULL) {
      printf("Hunt ID is required for '%s' operation\n", operation);
      exit(EXIT_FAILURE);
    }
    remove_hunt(hunt_id);
  } else {
    printf("Unknown operation '%s'\n", operation);
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
