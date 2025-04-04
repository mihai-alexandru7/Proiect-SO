#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "treasure_hunt.h"

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

int directory_exists(const char* pathname) {
  struct stat st;
  return (stat(pathname, &st) == 0 && S_ISDIR(st.st_mode));
}

void add_treasure(const char* treasure_hunt_id) {
  Treasure treasure = read_treasure_information();

  if (!directory_exists(treasure_hunt_id)) {
    if (mkdir(treasure_hunt_id, 0777) == -1) {
      perror("Error creating directory: ");
      return;
    }
  }

  char treasure_hunt_path[256];
  sprintf(treasure_hunt_path, "%s/treasures.dat", treasure_hunt_id);

  int file_descriptor = open(treasure_hunt_path, O_WRONLY | O_CREAT | O_APPEND, 0777);
  if (file_descriptor == -1) {
    perror("Error creating/opening file: ");
    return;
  }

  if (write(file_descriptor, &treasure, sizeof(treasure)) != sizeof(treasure)) {
    perror("Error writing to file: ");
    close(file_descriptor);
    return;
  }

  close(file_descriptor);
  
}
