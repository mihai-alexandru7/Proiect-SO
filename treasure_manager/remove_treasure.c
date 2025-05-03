#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "treasure.h"
#include "treasure_utils.h"

// Remove a treasure from a hunt
void remove_treasure(const char* hunt_id, const char* treasure_id) {
  // Check if the hunts directory exists
  if (!directory_exists(HUNTS_DIRECTORY)) {
    fprintf(stderr, "Hunts directory doesn't exist!");
    exit(EXIT_FAILURE);
  }
  // Create the relative path to the hunt directory
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
  int treasures_fd = open(treasures_file_path, O_RDWR);
  if (treasures_fd == -1) {
    perror("Error opening the treasures file");
    exit(EXIT_FAILURE);
  }
  // Remove the specified treasure from the hunt if it exists
  int found = 0;
  off_t read_offset = 0;
  off_t write_offset = 0;
  Treasure treasure;
  while (read(treasures_fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    if (strcmp(treasure.id, treasure_id) == 0) {
      found = 1;
      write_offset = read_offset;
    } else if (found) {
      lseek(treasures_fd, write_offset, SEEK_SET);
      if (write(treasures_fd, &treasure, sizeof(treasure)) != sizeof(treasure)) {
        perror("Error writing to the treasures file");
        close(treasures_fd);
        exit(EXIT_FAILURE);
      }
      lseek(treasures_fd, read_offset + sizeof(treasure), SEEK_SET);
      write_offset = write_offset + sizeof(treasure);
    }
    read_offset = read_offset + sizeof(treasure);
  }
  if (found) {
    if (ftruncate(treasures_fd, write_offset) == -1) {
      perror("Error truncating the treasures file");
    } else {
      printf("Treasure '%s' removed successfully from the hunt '%s'!\n", treasure_id, hunt_id);
    }
  } else {
    printf("Treasure '%s' wasn't found in the hunt '%s'!\n", treasure_id, hunt_id);
  }
  // Close treasures file
  if (close(treasures_fd) == -1) {
    perror("Error closing the treasures file");
  }
  // Log the operation
  log_remove_treasure_operation(treasure_id, hunt_id, found);
}
