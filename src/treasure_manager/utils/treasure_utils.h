#ifndef TREASURE_UTILS_H
#define TREASURE_UTILS_H

#include "../../../include/treasure.h"

#define MAX_PATH_SIZE 512
#define MAX_LOG_MESSAGE_LENGTH 256
#define HUNTS_DIRECTORY "hunts"
#define LOGS_DIRECTORY "logs"
#define HUNT_LOG_FILE "logged_hunt.txt"
#define TREASURES_FILE "treasures.dat"

// Reads treasure information from the standard input
Treasure read_treasure_information();
// Prints information about a treasure
void print_treasure_information(Treasure treasure);
// Checks if the file exists
int file_exists(const char* file_path);
// Checks if the directory exists
int directory_exists(const char* directory_path);
// Checks if the symbolic link exists
int symlink_exists(const char* symlink_path);
// Creates the symbolic link in the logs directory
void create_symlink(const char* symlink_path, const char* file_path);
// Logs the operation message to a file
void log_operation(const char* log_file_path, const char* log_message);
// Logs the creation of a hunt and creates the symlink
void log_create_hunt_operation(const char* hunt_id);
// Logs the addition of a treasure to a hunt
void log_add_treasure_operation(const char* treasure_id, const char* hunt_id, int success);
// Logs the viewing of a treasure from a specified hunt
void log_view_treasure_operation(const char* treasure_id, const char* hunt_id, int found);
// Logs the listing of all the treasures from a hunt
void log_list_treasures_operation(const char* hunt_id, int count);
// Logs the removal of a treasure from a hunt
void log_remove_treasure_operation(const char* treasure_id, const char* hunt_id, int found);
// Logs the removal of a hunt
void log_remove_hunt_operation(const char* hunt_id, int success);

#endif
