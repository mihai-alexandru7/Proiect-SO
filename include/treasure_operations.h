#ifndef TREASURE_OPERATIONS_H
#define TREASURE_OPERATIONS_H

// Add a new treasure to the hunt
void add_treasure(const char* hunt_id);
// View details of a treasure within a hunt
void view_treasure(const char* hunt_id, const char* treasure_id);
// List all treasures in the hunt
void list_treasures(const char* hunt_id);
// List all available hunts
void list_hunts();
// Remove a treasure from a hunt
void remove_treasure(const char* hunt_id, const char* treasure_id);
// Remove a hunt
void remove_hunt(const char* hunt_id);

#endif
