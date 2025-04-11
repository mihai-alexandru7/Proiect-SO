#ifndef TREASURE_HUNT_H
#define TREASURE_HUNT_H

#define MAX_ID_LENGTH 64
#define MAX_USERNAME_LENGTH 64
#define MAX_CLUE_LENGTH 128

typedef struct {
  char id[MAX_ID_LENGTH];
  char username[MAX_USERNAME_LENGTH];
  struct {
    double latitude;
    double longitude;
  } GPS;
  char clue[MAX_CLUE_LENGTH];
  int value;
} Treasure;

void add_treasure(const char* treasure_hunt_id);
void list_treasure_hunt(const char* treasure_hunt_id);
void view_treasure(const char* treasure_hunt_id, const char* treasure_id);
void remove_treasure(const char* treasure_hunt_id, const char* treasure_id);
void remove_treasure_hunt(const char* treasure_hunt_id);

#endif
