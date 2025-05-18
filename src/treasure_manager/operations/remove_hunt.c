#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "../../../include/treasure.h"
#include "../utils/treasure_utils.h"

void remove_hunt(const char* hunt_id) {
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
  // Remove the treasures file if it exists
  if (file_exists(treasures_file_path)) {
    if (unlink(treasures_file_path) == -1) {
      perror("Error removing the treasures file");
      // Log the operation
      log_remove_hunt_operation(hunt_id, 0);
      exit(EXIT_FAILURE);
    }
  }
  // Create the relative path to the symbolic link to the log file
  char log_file_symlink_path[MAX_PATH_SIZE];
  snprintf(log_file_symlink_path, sizeof(log_file_symlink_path), "%s/logged_hunt-%s", LOGS_DIRECTORY, hunt_id);
  // Remove the symbolic link to the log file if it exists
  if (unlink(log_file_symlink_path) == -1) {
    perror("Error removing the symbolic link to the log file");
  }
  // Create the relative path to the log file
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, HUNT_LOG_FILE);
  // Create the new relative path to the log file
  char log_file_new_path[MAX_PATH_SIZE];
  snprintf(log_file_new_path, sizeof(log_file_new_path), "%s/%s-%s(removed).txt", LOGS_DIRECTORY, "logged_hunt", hunt_id);
  // Move the log file to the logs directory if it exists
  if (directory_exists(LOGS_DIRECTORY)) {
    if (rename(log_file_path, log_file_new_path) == -1) {
      perror("Error moving the log file");
    }
  }
  // Remove the hunt directory
  if (rmdir(hunt_dir_path) == -1) {
    perror("Error removing the hunt directory");
    // Log the operation
    log_remove_hunt_operation(hunt_id, 0);
    exit(EXIT_FAILURE);
  }
  printf("Hunt '%s' removed successfully!\n", hunt_id);
  // Log the operation
  log_remove_hunt_operation(hunt_id, 1);
}
