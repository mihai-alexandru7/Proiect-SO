#ifndef TREASURE_H
#define TREASURE_H

#define MAX_ID_LENGTH 64
#define MAX_USERNAME_LENGTH 64
#define MAX_CLUE_LENGTH 128

typedef struct {
  double latitude;
  double longitude;
} Coordinate;

typedef struct {
  char id[MAX_ID_LENGTH];
  char username[MAX_USERNAME_LENGTH];
  Coordinate coordinate;
  char clue[MAX_CLUE_LENGTH];
  int value;
} Treasure;

#endif
