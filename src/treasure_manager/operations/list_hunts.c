#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include "../../../include/treasure.h"
#include "../utils/treasure_utils.h"

void list_hunts() {
  // Check if the hunts directory exists
  if (!directory_exists(HUNTS_DIRECTORY)) {
    fprintf(stderr, "Hunts directory doesn't exist!");
    exit(EXIT_FAILURE);
  }
  // Open the hunts directory
  DIR* dir = opendir(HUNTS_DIRECTORY);
  if (!dir) {
    perror("Error opening the hunts directory");
    exit(EXIT_FAILURE);
  }
  // Read and print all the hunts
  int count = 0;
  struct dirent* entry;
  while ((entry = readdir(dir))) {
    // Skip "." and ".." entries
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    // Create the relative path to the entry
    char hunt_dir_path[MAX_PATH_SIZE];
    snprintf(hunt_dir_path, sizeof(hunt_dir_path), "%s/%s", HUNTS_DIRECTORY, entry->d_name);
    // Check if the entry it's a directory
    if (directory_exists(hunt_dir_path)) {
      if (!count) {
        printf("The following hunts are available:\n");
      }
      count++;
      printf("- Hunt %d: %s\n", count, entry->d_name);
    }
  }
  // Close the hunts directory
  if (closedir(dir) == -1) {
    perror("Error closing the hunts directory");
  }
  if (count == 0) {
    printf("No hunts found\n");
  }
}
