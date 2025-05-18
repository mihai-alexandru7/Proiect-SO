#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "../../../include/treasure.h"
#include "../utils/treasure_utils.h"

void view_treasure(const char* hunt_id, const char* treasure_id) {
  // Check if the hunts directory exists
  if (!directory_exists(HUNTS_DIRECTORY)) {
    fprintf(stderr, "Hunts directory doesn't exist!");
    exit(EXIT_FAILURE);
  }
  // Create the relative path to hunt directory
  char hunt_dir_path[MAX_PATH_SIZE];
  snprintf(hunt_dir_path, sizeof(hunt_dir_path), "%s/%s", HUNTS_DIRECTORY, hunt_id);
  // Check if the hunt exists
  if (!directory_exists(hunt_dir_path)) {
    fprintf(stderr, "Hunt '%s' doesn't exist!\n", hunt_id);
    exit(EXIT_FAILURE);
  }
  // Create the relative path to the treasures file
  char treasures_file_path[MAX_PATH_SIZE];
  snprintf(treasures_file_path, sizeof(treasures_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, TREASURES_FILE);
  // Check if the treasures file exists
  if (!file_exists(treasures_file_path)) {
    fprintf(stderr, "Treasures file doesn't exist for the hunt '%s'!\n", hunt_id);
    exit(EXIT_FAILURE);
  }
  // Open the treasures file
  int treasures_fd = open(treasures_file_path, O_RDONLY);
  if (treasures_fd == -1) {
    perror("Error opening the treasures file");
    exit(EXIT_FAILURE);
  }
  // Print the information about the specified treasure to the standard output if it is found in the hunt
  int found = 0;
  Treasure treasure;
  while (read(treasures_fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    if (strcmp(treasure.id, treasure_id) == 0) {
      printf("Treasure '%s' was found in the hunt '%s'!\n", treasure_id, hunt_id);
      print_treasure_information(treasure);
      found = 1;
      break;
    }
  }
  // Close the treasures file
  if (close(treasures_fd) == -1) {
    perror("Error closing the treasures file");
  }
  if (!found) {
    printf("Treasure '%s' wasn't found in the hunt '%s'!\n", treasure_id, hunt_id);
  }
  // Log the operation
  log_view_treasure_operation(treasure_id, hunt_id, found);
}
