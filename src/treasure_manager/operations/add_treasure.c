#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../../../include/treasure.h"
#include "../utils/treasure_utils.h"

void add_treasure(const char* hunt_id) {
  // Create the hunts directory if it doesn't exist
  if (!directory_exists(HUNTS_DIRECTORY)) {
    if (mkdir(HUNTS_DIRECTORY, 0755) == -1) {
      perror("Error creating the hunts directory");
      exit(EXIT_FAILURE);
    }
  }
  // Create the relative path to the hunt directory
  char hunt_dir_path[MAX_PATH_SIZE];
  snprintf(hunt_dir_path, sizeof(hunt_dir_path),"%s/%s", HUNTS_DIRECTORY, hunt_id);
  // Create the hunt if it doesn't exist
  if (!directory_exists(hunt_dir_path)) {
    if (mkdir(hunt_dir_path, 0755) == -1) {
      perror("Error creating the hunt directory");
      exit(EXIT_FAILURE);
    }
    // Log the hunt creation
    log_create_hunt_operation(hunt_id);
  }
  // Create the relative path to the treasures file
  char treasures_file_path[MAX_PATH_SIZE];
  snprintf(treasures_file_path, sizeof(treasures_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, TREASURES_FILE);
  // Open or create the treasures file
  int treasures_fd = open(treasures_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (treasures_fd == -1) {
    perror("Error opening/creating the treasures file");
    exit(EXIT_FAILURE);
  }
  // Read the information about the treasure from the standard input and write it to the treasures file
  Treasure treasure = read_treasure_information();
  ssize_t bytes_written = write(treasures_fd, &treasure, sizeof(treasure));
  // Close the treasures file
  if (close(treasures_fd) == -1) {
    perror("Error closing the treasures file");
  }
  // Check if the treasure was correctly written
  if (bytes_written != sizeof(treasure)) {
    perror("Error writing to the treasure file");
    // Log the operation
    log_add_treasure_operation(treasure.id, hunt_id, 0);
    exit(EXIT_FAILURE);
  }
  printf("Treasure '%s' added successfully to the hunt '%s'!\n", treasure.id, hunt_id);
  // Log the operation
  log_add_treasure_operation(treasure.id, hunt_id, 1);
}
