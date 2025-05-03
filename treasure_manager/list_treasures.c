#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "treasure.h"
#include "treasure_utils.h"

// List all treasures in a hunt
void list_treasures(const char* hunt_id) {
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
    fprintf(stderr, "Treasures file doesn't exist for hunt '%s'!\n", hunt_id);
    exit(EXIT_FAILURE);
  }
  // Get the treasures file attributes
  struct stat st;
  if (stat(treasures_file_path, &st) == -1) {
    perror("Error retrieving the treasures file attributes");
    exit(EXIT_FAILURE);
  }
  // Print the hunt information
  printf("Hunt name: %s\n", hunt_id);
  printf("Total treasures file size: %ld bytes\n", st.st_size);
  printf("Last modification time of treasures file: %s", ctime(&st.st_mtime));
  // Open the treasures file
  int treasures_fd = open(treasures_file_path, O_RDONLY);
  if (treasures_fd == -1) {
    perror("Error opening the treasures file");
    exit(EXIT_FAILURE);
  }
  // Read and print all the treasures from the hunt
  int count = 0;
  Treasure treasure;
  while (read(treasures_fd, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    if(!count) {
      printf("The hunt contains the following treasures:\n");
    }
    count++;
    printf("--- Treasure %d ---\n", count);
    print_treasure_information(treasure);
  }
  // Close the treasures file
  if (close(treasures_fd) == -1) {
    perror("Error closing the treasures file");
  }
  if (!count) {
    printf("No treasures found in the hunt '%s'\n", hunt_id);
  }
  // Log the operation
  log_list_treasures_operation(hunt_id, count);
}
