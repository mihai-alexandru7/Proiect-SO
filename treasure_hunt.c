#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include "treasure_hunt.h"

#define MAX_PATH_SIZE 256

Treasure read_treasure_information() {
  Treasure treasure;
  
  printf("Enter treasure ID: ");
  fgets(treasure.id, MAX_ID_LENGTH, stdin);
  treasure.id[strcspn(treasure.id, "\n")] = '\0';

  printf("Enter username: ");
  fgets(treasure.username, MAX_USERNAME_LENGTH, stdin);
  treasure.username[strcspn(treasure.username, "\n")] = '\0';
  
  printf("Enter GPS latitude coordinate: ");
  scanf("%lf", &treasure.GPS.latitude);
  getchar();
  printf("Enter GPS longitude coordinate: ");
  scanf("%lf", &treasure.GPS.longitude);
  getchar();

  printf("Enter treasure clue: ");
  fgets(treasure.clue, MAX_CLUE_LENGTH, stdin);
  treasure.clue[strcspn(treasure.clue, "\n")] = '\0';

  printf("Enter treasure value: ");
  scanf("%d", &treasure.value);
  getchar();

  return treasure;
}

int file_exists(const char* file_name) {
  struct stat st;
  return (stat(file_name, &st) == 0 && S_ISREG(st.st_mode) != 0);
}

int directory_exists(const char* pathname) {
  struct stat st;
  return (stat(pathname, &st) == 0 && S_ISDIR(st.st_mode) != 0);
}

void write_treasure_information_to_file(int file_descriptor, Treasure treasure) {
  if (write(file_descriptor, &treasure, sizeof(treasure)) != sizeof(treasure)) {
    perror("Error writing to treasure file: ");
    close(file_descriptor);
  }
}

void add_treasure(const char* treasure_hunt_id) {
  if (!directory_exists(treasure_hunt_id)) {
    if (mkdir(treasure_hunt_id, 0755) == -1) {
      perror("Error creating treasure hunt directory: ");
      return;
    }
  }

  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  int file_descriptor = open(treasure_file_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (file_descriptor == -1) {
    perror("Error creating/opening treasure file: ");
    return;
  }

  char answer[5];
  do {
    Treasure treasure = read_treasure_information();
    write_treasure_information_to_file(file_descriptor, treasure);
    printf("Treasure '%s' added succesfully\n", treasure.id);
    printf("Do you want to add another treasure? (YES/NO)\n");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = '\0';
  } while(strcmp(answer, "YES") == 0 || strcmp(answer, "yes") == 0);

  close(file_descriptor);
}

void print_treasure_information(Treasure treasure) {
  printf("Treasure ID: %s\n", treasure.id);
  printf("Username: %s\n", treasure.username);
  printf("GPS latitude coordinate: %.2f\n", treasure.GPS.latitude);
  printf("GPS longitude coordinate: %.2f\n", treasure.GPS.longitude);
  printf("Clue: %s\n", treasure.clue);
  printf("Value: %d\n", treasure.value);
}

void list_treasure_hunt(const char* treasure_hunt_id) {
  if (!directory_exists(treasure_hunt_id)) {
    printf("The '%s' treasure hunt does not exist!\n", treasure_hunt_id);
    return;
  }
  
  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);
  
  struct stat st;
  if (stat(treasure_file_path, &st) == -1) {
    perror("Error retrieving treasure file attributes: ");
    return;
  }

  printf("Treasure Hunt name: %s\n", treasure_hunt_id);
  printf("Total treasure file size: %ld bytes\n", st.st_size);
  printf("Last modification time of treasure file: %s", ctime(&st.st_mtime));

  int file_descriptor = open(treasure_file_path, O_RDONLY);
  if (file_descriptor == -1) {
    perror("Error opening treasure file: ");
    return;
  }

  printf("The treasure file contains the following treasures:\n");
  
  Treasure treasure;
  while (read(file_descriptor, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    printf("\n");
    print_treasure_information(treasure);
  }

  close(file_descriptor);
}

void view_treasure(const char* treasure_hunt_id, const char* treasure_id) {
  if (!directory_exists(treasure_hunt_id)) {
    printf("The '%s' treasure hunt does not exist!\n", treasure_hunt_id);
    return;
  }

  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  if (!file_exists(treasure_file_path)) {
    printf("The '%s' treasure file does not exist!\n", treasure_file_path);
    return;
  }

  int file_descriptor = open(treasure_file_path, O_RDONLY);
  if (file_descriptor == -1) {
    perror("Error opening treasure file: ");
    return;
  }

  int found = 0;
  Treasure treasure;
  while (read(file_descriptor, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    if (strcmp(treasure.id, treasure_id) == 0) {
      print_treasure_information(treasure);
      found = 1;
      break;
    }
  }

  close(file_descriptor);
  
  if (!found) {
    printf("Treasure '%s' was not found in treasure hunt '%s'!\n", treasure_id, treasure_hunt_id);
  }
}
