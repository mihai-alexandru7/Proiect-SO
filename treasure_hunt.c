#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include "treasure_hunt.h"

#define MAX_PATH_SIZE 128
#define MAX_LOG_MESSAGE_LENGTH 256

int file_exists(const char* file_path) {
  struct stat st;
  return (stat(file_path, &st) == 0 && S_ISREG(st.st_mode) != 0);
}

int directory_exists(const char* directory_path) {
  struct stat st;
  return (stat(directory_path, &st) == 0 && S_ISDIR(st.st_mode) != 0);
}

void log_operation(const char* treasure_hunt_id, const char* log_message) {
  char log_file_path[MAX_PATH_SIZE];
  sprintf(log_file_path, "%s/logged_treasure_hunt.txt", treasure_hunt_id);
  
  int log_file_descriptor = open(log_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (log_file_descriptor == -1) {
    perror("Error opening/creating log file: ");
    return;
  }
  
  if (write(log_file_descriptor, log_message, strlen(log_message)) != strlen(log_message)) {
    perror("Error writing to log file: ");
    close(log_file_descriptor);
    return;
  }
  
  close(log_file_descriptor);
  
  char log_file_symbolic_link[MAX_PATH_SIZE];
  sprintf(log_file_symbolic_link, "logged_hunt-%s", treasure_hunt_id);
  
  struct stat st;
  if (lstat(log_file_symbolic_link, &st) == -1) {
    if (symlink(log_file_path, log_file_symbolic_link) == -1) {
      perror("Error creating symbolic link: ");
      return;
    }
  }
}

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

void add_treasure(const char* treasure_hunt_id) {
  if (!directory_exists(treasure_hunt_id)) {
    if (mkdir(treasure_hunt_id, 0755) == -1) {
      perror("Error creating treasure hunt directory: ");
      return;
    }
    char log_message[MAX_PATH_SIZE];
    sprintf(log_message, "Created treasure hunt '%s'\n", treasure_hunt_id);
    log_operation(treasure_hunt_id, log_message);
  }

  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  int file_descriptor = open(treasure_file_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (file_descriptor == -1) {
    perror("Error opening/creating treasure file: ");
    return;
  }

  char answer[5];
  do {
    Treasure treasure = read_treasure_information();
    if (write(file_descriptor, &treasure, sizeof(treasure)) != sizeof(treasure)) {
      perror("Error writing to treasure file: ");
      close(file_descriptor);
      return;
    }
    printf("Treasure '%s' added succesfully\n", treasure.id);
    char log_message[MAX_LOG_MESSAGE_LENGTH];
    sprintf(log_message, "Added treasure '%s' to treasure hunt '%s'\n", treasure.id, treasure_hunt_id);
    log_operation(treasure_hunt_id, log_message);
    printf("Do you want to add another treasure to this treasure hunt? (YES/NO): ");
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
    printf("The '%s' treasure hunt doesn't exist!\n", treasure_hunt_id);
    return;
  }
  
  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  if (!file_exists(treasure_file_path)) {
    printf("The '%s' treasure file doesn't exist!\n", treasure_file_path);
    return;
  }
  
  struct stat st;
  if (stat(treasure_file_path, &st) == -1) {
    perror("Error retrieving treasure file attributes: ");
    return;
  }

  printf("Treasure Hunt name: %s\n", treasure_hunt_id);
  printf("Total treasure file size: %ld bytes\n", st.st_size);
  printf("Last modification time of treasure file: %s\n", ctime(&st.st_mtime));

  int file_descriptor = open(treasure_file_path, O_RDONLY);
  if (file_descriptor == -1) {
    perror("Error opening treasure file: ");
    return;
  }

  printf("The treasure file contains the following treasures:\n");

  int count = 0;
  Treasure treasure;
  while (read(file_descriptor, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    count++;
    printf("--- %d ---\n", count);
    print_treasure_information(treasure);
  }
  
  close(file_descriptor);

  char log_message[MAX_LOG_MESSAGE_LENGTH];
  sprintf(log_message, "Listed all treasures from treasure hunt '%s'\n", treasure_hunt_id);
  log_operation(treasure_hunt_id, log_message);
}

void view_treasure(const char* treasure_hunt_id, const char* treasure_id) {
  if (!directory_exists(treasure_hunt_id)) {
    printf("The '%s' treasure hunt doesn't exist!\n", treasure_hunt_id);
    return;
  }

  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  if (!file_exists(treasure_file_path)) {
    printf("The '%s' treasure file doesn't exist!\n", treasure_file_path);
    return;
  }

  int file_descriptor = open(treasure_file_path, O_RDONLY);
  if (file_descriptor == -1) {
    perror("Error opening treasure file: ");
    return;
  }

  char log_message[MAX_LOG_MESSAGE_LENGTH];

  int found = 0;
  Treasure treasure;
  while (read(file_descriptor, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    if (strcmp(treasure.id, treasure_id) == 0) {
      print_treasure_information(treasure);
      sprintf(log_message, "Viewed treasure '%s' from treasure hunt '%s'\n", treasure_id, treasure_hunt_id);
      found = 1;
      break;
    }
  }

  close(file_descriptor);
  
  if (!found) {
    printf("Treasure '%s' wasn't found in treasure hunt '%s'!\n", treasure_id, treasure_hunt_id);
    sprintf(log_message, "Failed to view treasure '%s' from treasure hunt '%s'\n", treasure_id, treasure_hunt_id);
  }

  log_operation(treasure_hunt_id, log_message);
}

void remove_treasure(const char* treasure_hunt_id, const char* treasure_id) {
  if (!directory_exists(treasure_hunt_id)) {
    printf("Treasure hunt '%s' doesn't exist!\n", treasure_hunt_id);
    return;
  }

  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  if (!file_exists(treasure_file_path)) {
    printf("Treasure file '%s' doesn't exist!\n", treasure_file_path);
    return;
  }

  int file_descriptor = open(treasure_file_path, O_RDWR);
  if (file_descriptor == -1) {
    perror("Error opening treasure file: ");
    return;
  }

  int found = 0;
  off_t read_offset = 0;
  off_t write_offset = 0;
  Treasure treasure;
  while (read(file_descriptor, &treasure, sizeof(treasure)) == sizeof(treasure)) {
    if (strcmp(treasure.id, treasure_id) == 0) {
      found = 1;
      write_offset = read_offset;
    } else if (found) {
      lseek(file_descriptor, write_offset, SEEK_SET);
      if (write(file_descriptor, &treasure, sizeof(treasure)) != sizeof(treasure)) {
	perror("Error writing to treasure file: ");
	close(file_descriptor);
	return;
      }
      lseek(file_descriptor, read_offset + sizeof(treasure), SEEK_SET);
      write_offset = write_offset + sizeof(treasure);
    }
    read_offset = read_offset + sizeof(treasure);
  }

  char log_message[MAX_LOG_MESSAGE_LENGTH];
  if (found) {
    if (ftruncate(file_descriptor, write_offset) == -1) {
      perror("Error truncating treasure file: ");
    } else {
      printf("Treasure '%s' removed successfully from treasure hunt '%s'!\n", treasure_id, treasure_hunt_id);
      sprintf(log_message, "Removed treasure '%s' from treasure hunt '%s'\n", treasure_id, treasure_hunt_id);
    }
  } else {
    printf("Treasure '%s' wasn't found in treasure hunt '%s'!\n", treasure_id, treasure_hunt_id);
    sprintf(log_message, "Failed to remove treasure '%s' from treasure hunt '%s'\n", treasure_id, treasure_hunt_id);
  }
  log_operation(treasure_hunt_id, log_message);

  close(file_descriptor);
}

void remove_treasure_hunt(const char* treasure_hunt_id) {
  if (!directory_exists(treasure_hunt_id)) {
    printf("Treasure hunt '%s' doesn't exist!\n", treasure_hunt_id);
    return;
  }

  char treasure_file_path[MAX_PATH_SIZE];
  sprintf(treasure_file_path, "%s/treasures.dat", treasure_hunt_id);

  if (!file_exists(treasure_file_path)) {
    printf("Treasure file '%s' doesn't exist!\n", treasure_file_path);
    return;
  }

  if (unlink(treasure_file_path) == -1) {
    perror("Error removing treasure file: ");
    return;
  }

  char log_message[MAX_LOG_MESSAGE_LENGTH];
  printf("Treasure hunt '%s' removed successfully!\n", treasure_hunt_id);
  sprintf(log_message, "Removed treasure hunt '%s'\n", treasure_hunt_id);
  log_operation(treasure_hunt_id, log_message);
}
