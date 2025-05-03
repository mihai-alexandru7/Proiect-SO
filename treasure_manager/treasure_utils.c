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

// Reads treasure information from the standard input
Treasure read_treasure_information() {
  Treasure treasure;
  // Read treasure ID
  printf("Enter the treasure ID: ");
  if (!fgets(treasure.id, MAX_ID_LENGTH, stdin)) {
    perror("Error reading the treasure ID");
    exit(EXIT_FAILURE);
  }
  treasure.id[strcspn(treasure.id, "\n")] = '\0';
  // Read username
  printf("Enter the username: ");
  if(!fgets(treasure.username, MAX_USERNAME_LENGTH, stdin)) {
    perror("Error reading the username");
    exit(EXIT_FAILURE);
  }
  treasure.username[strcspn(treasure.username, "\n")] = '\0';
  // Read latitude and longitude
  printf("Enter the GPS latitude coordinate: ");
  if (scanf("%lf", &treasure.coordinate.latitude) != 1) {
    perror("Error reading the latitude coordinate for treasure");
    exit(EXIT_FAILURE);
  }
  getchar();
  printf("Enter the GPS longitude coordinate: ");
  if (scanf("%lf", &treasure.coordinate.longitude) != 1) {
    perror("Error reading the longitude coordinate for treasure");
    exit(EXIT_FAILURE);
  }
  getchar();
  // Read treasure clue
  printf("Enter the treasure clue: ");
  if (!fgets(treasure.clue, MAX_CLUE_LENGTH, stdin)) {
    perror("Error reading the treasure clue");
    exit(EXIT_FAILURE);
  }
  treasure.clue[strcspn(treasure.clue, "\n")] = '\0';
  // Read treasure value
  printf("Enter the treasure value: ");
  if (scanf("%d", &treasure.value) != 1) {
    perror("Error reading the treasure value");
    exit(EXIT_FAILURE);        ;
  }
  getchar();
  return treasure;
}

// Prints information about a treasure
void print_treasure_information(Treasure treasure) {
  printf("Treasure ID: %s\n", treasure.id);
  printf("Username: %s\n", treasure.username);
  printf("GPS latitude coordinate: %.2f\n", treasure.coordinate.latitude);
  printf("GPS longitude coordinate: %.2f\n", treasure.coordinate.longitude);
  printf("Clue: %s\n", treasure.clue);
  printf("Value: %d\n", treasure.value);
}

// Checks if the file exists
int file_exists(const char* file_path) {
  struct stat st;
  return (stat(file_path, &st) == 0 && S_ISREG(st.st_mode) != 0);
}

// Checks if the directory exists
int directory_exists(const char* directory_path) {
  struct stat st;
  return (stat(directory_path, &st) == 0 && S_ISDIR(st.st_mode) != 0);
}

// Checks if the symbolic link exists
int symlink_exists(const char* symlink_path) {
  struct stat st;
  return(lstat(symlink_path, &st) == 0 && S_ISLNK(st.st_mode) != 0);
}

// Creates the symbolic link in the logs directory
void create_symlink(const char* symlink_path, const char* file_path) {
  // Create the logs directory if it doesn't exist
  if (!directory_exists(LOGS_DIRECTORY)) {
    if (mkdir(LOGS_DIRECTORY, 0755) == -1) {
      perror("Error creating logs directory");
      return;
    }
  }
  // Create the symbolic link if it doesn't exist
  if (!symlink_exists(symlink_path)) {
    if (symlink(file_path, symlink_path) == -1) {
      perror("Error creating symbolic link");
    }
  }
}

// Logs the operation message to a file
void log_operation(const char* log_file_path, const char* log_message) {
  // Add timestamp to the log message
  char timestamped_log_message[MAX_LOG_MESSAGE_LENGTH];
  time_t now = time(NULL);
  snprintf(timestamped_log_message, sizeof(timestamped_log_message), "%s%s\n\n", ctime(&now), log_message);
  // Open or create the log file
  int log_fd = open(log_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (log_fd == -1) {
    perror("Error opening/creating log file");
    return;
  }
  // Write the log message to the log file
  ssize_t bytes_written = write(log_fd, timestamped_log_message, strlen(timestamped_log_message));
  // Close the log file
  if (close(log_fd) == -1) {
    perror("Error closing log file");
  }
  // Check if the log message was correctly written
  if (bytes_written != (ssize_t)strlen(timestamped_log_message)) {
    perror("Error writing to the log file");
  }
}

// Logs the creation of a hunt and creates the symlink
void log_create_hunt_operation(const char* hunt_id) {
  // Create the log message for creating a hunt
  char log_message[MAX_LOG_MESSAGE_LENGTH];
  snprintf(log_message, sizeof(log_message), "Created hunt '%s'", hunt_id);
  // Create the relative path to the log file of the specified hunt
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, HUNT_LOG_FILE);
  // Log the operation
  log_operation(log_file_path, log_message);
  // Create the relative path to the symbolic link to the log file in the logs directory
  char log_file_symlink_path[MAX_PATH_SIZE];
  snprintf(log_file_symlink_path, sizeof(log_file_symlink_path), "%s/logged_hunt-%s", LOGS_DIRECTORY, hunt_id);
  // Create the symbolic link to the log file of the specified hunt
  create_symlink(log_file_symlink_path, log_file_path);
}

// Logs the addition of a treasure to a hunt
void log_add_treasure_operation(const char* treasure_id, const char* hunt_id, int success) {
  // Create the log message for adding a treasure
  char log_message[MAX_LOG_MESSAGE_LENGTH];
  if (success) {
    snprintf(log_message, sizeof(log_message), "Added treasure '%s' to hunt '%s'", treasure_id, hunt_id);
  } else {
    snprintf(log_message, sizeof(log_message), "Attempted to add treasure '%s' to hunt '%s'", treasure_id, hunt_id);
  }
  // Create the relative path to the log file of the specified hunt
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, HUNT_LOG_FILE);
  // Log the operation
  log_operation(log_file_path, log_message);
}

// Logs the viewing of a treasure from a specified hunt
void log_view_treasure_operation(const char* treasure_id, const char* hunt_id, int found) {
  // Create the log message for viewing a treasure from a hunt
  char log_message[MAX_LOG_MESSAGE_LENGTH];
  if (found) {
    snprintf(log_message, sizeof(log_message), "Viewed treasure '%s' from hunt '%s'", treasure_id, hunt_id);
  } else {
    snprintf(log_message, sizeof(log_message), "Attempted to view non-existent treasure '%s' from hunt '%s'", treasure_id, hunt_id);
  }
  // Create the relative path to the log file of the specified hunt
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, HUNT_LOG_FILE);
  // Log the operation
  log_operation(log_file_path, log_message);
}

// Logs the listing of all the treasures from a hunt
void log_list_treasures_operation(const char* hunt_id, int count) {
  // Create the log message for listing all the treasures from a hunt
  char log_message[MAX_LOG_MESSAGE_LENGTH];
  if (count) {
    snprintf(log_message, sizeof(log_message), "Listed all treasures from the hunt '%s'", hunt_id);
  } else {
    snprintf(log_message, sizeof(log_message), "No treasures the hunt '%s'", hunt_id);
  }
  // Create the relative path to the log file of the specified hunt
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, HUNT_LOG_FILE);
  // Log the operation
  log_operation(log_file_path, log_message);
}

// Logs the removal of a treasure from a hunt
void log_remove_treasure_operation(const char* treasure_id, const char* hunt_id, int found) {
  // Create the log message for removing a treasure from a hunt
  char log_message[MAX_LOG_MESSAGE_LENGTH];
  if (found) {
    snprintf(log_message, sizeof(log_message), "Removed treasure '%s' from hunt '%s'", treasure_id, hunt_id);
  } else {
    snprintf(log_message, sizeof(log_message), "Attempted to remove non-existent treasure '%s' from hunt '%s'", treasure_id, hunt_id);
  }
  // Create the relative path to the log file of the specified hunt
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, HUNT_LOG_FILE);
  // Log the operation
  log_operation(log_file_path, log_message);
}

// Logs the removal of a hunt
void log_remove_hunt_operation(const char* hunt_id, int success) {
  // Create the log message for removing a hunt
  char log_message[MAX_LOG_MESSAGE_LENGTH];
  if (success) {
    snprintf(log_message, sizeof(log_message), "Removed hunt '%s'", hunt_id);
  } else {
    snprintf(log_message, sizeof(log_message), "Attempted to remove non-existent hunt '%s'", hunt_id);
  }
  // Create the relative path to the log file of the removed hunt
  char log_file_path[MAX_PATH_SIZE];
  snprintf(log_file_path, sizeof(log_file_path), "%s/logged_hunt-%s(removed).txt", LOGS_DIRECTORY, hunt_id);
  // Log the operation
  log_operation(log_file_path, log_message);
}
