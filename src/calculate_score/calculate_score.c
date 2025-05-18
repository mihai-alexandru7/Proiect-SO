#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "../../include/treasure.h"

#define HUNTS_DIRECTORY "hunts"
#define TREASURES_FILE "treasures.dat"
#define MAX_PATH_SIZE 512
#define HASH_TABLE_SIZE 23

typedef struct User {
  char name[MAX_USERNAME_LENGTH];
  int score;
  struct User* next;
} User;

User* users[HASH_TABLE_SIZE] = {NULL};

unsigned int hash(const char* str) {
  unsigned int hash_value = 0;
  while (*str) {
    hash_value = hash_value * 31 + (unsigned char)(*str);
    str++;
  }
  return hash_value % HASH_TABLE_SIZE;
}

User* create_node(const char* name, int score) {
  User* new_user = (User*)malloc(sizeof(User));
  if (!new_user) {
    perror("Error allocating the memory for the node");
    return NULL;
  }
  strcpy(new_user->name, name);
  new_user->score = score;
  new_user->next = NULL;
  return new_user;
}

void update_user_score(const char* name, int score) {
  unsigned int index = hash(name);
  //
  User* current_user = users[index];
  while (current_user != NULL) {
    if (strcmp(current_user->name, name) == 0) {
      current_user->score = current_user->score + score;
      return;
    }
    current_user = current_user->next;
  }
  //
  User* new_user = create_node(name, score);
  if (!new_user) {
    perror("Error creating the node");
    return;
  }
  // Insert at the beginning of the chain
  new_user->next = users[index];
  users[index] = new_user;
}

void print_user_score(User user) {
  printf("User's name: %s\n", user.name);
  printf("User's score: %d\n", user.score);
}

void print_users_scores() {
  int users_found = 0;
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    User* current_user = users[i];
    while (current_user != NULL) {
      users_found++;
      printf("--- User %d ---\n", users_found);
      print_user_score(*current_user);
      current_user = current_user->next;
    }
  }
  if (users_found == 0) {
    printf("No users found in this hunt\n");
  }
}

void free_hash_table() {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    User* current_user = users[i];
    while (current_user) {
      User* temp = current_user;
      current_user = current_user->next;
      free(temp);
    }
    users[i] = NULL;
  }
}

void process_treasures(const char* hunt_id) {
  // Create the relative path to the hunt directory
  char hunt_directory_path[MAX_PATH_SIZE];
  snprintf(hunt_directory_path, MAX_PATH_SIZE, "%s/%s", HUNTS_DIRECTORY, hunt_id);
  // Check if the hunts directory exists
  struct stat st;
  if (stat(hunt_directory_path, &st) == -1 || S_ISDIR(st.st_mode) == 0) {
    fprintf(stderr, "Hunt '%s' doesn't exist!\n", hunt_id);
    return;
  }
  // Create the relative path to the treasures file
  char treasures_file_path[MAX_PATH_SIZE];
  snprintf(treasures_file_path, MAX_PATH_SIZE, "%s/%s/%s", HUNTS_DIRECTORY, hunt_id, TREASURES_FILE);
  // Open the treasures file
  int treasures_fd = open(treasures_file_path, O_RDONLY);
  if (treasures_fd == -1) {
    perror("Error opening the treasures file");
    return;
  }
  // Read treasures from the file and update scores
  Treasure treasure;
  ssize_t bytes_read;
  while ((bytes_read = read(treasures_fd, &treasure, sizeof(Treasure))) == sizeof(Treasure)) {
    // Add the treasure score to the user's score
    update_user_score(treasure.username, treasure.value);
  }
  // Close the treasures file
  if (close(treasures_fd) == -1) {
    perror("Error closing the treasures file");
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
    return EXIT_FAILURE;
  }
  char* hunt_id = argv[1];
  process_treasures(hunt_id);
  // Print the scores
  printf("Scores of users for hunt '%s'\n", hunt_id);
  print_users_scores();
  free_hash_table();  
  return EXIT_SUCCESS;
}