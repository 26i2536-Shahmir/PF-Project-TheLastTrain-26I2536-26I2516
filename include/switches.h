#ifndef SWITCHES_H
#define SWITCHES_H

#include "state.h"

// Access control & puzzle validation without structs
bool islift2floor(int floor);
bool canuselift1(int fromfloor, int tofloor);
bool canuselift2(int fromfloor, int tofloor, bool lift2operational, int emergencytripsleft);

bool unlockfloor3door(const char inventory[MAX_INVENTORY_CAPACITY], int inventorycount);
bool installfusecontrolpanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount,
                             bool objectives[NUM_OBJECTIVES], int& score);
bool verifycabinetcode(const char* enteredcode, const bool objectives[NUM_OBJECTIVES]);

#endif // SWITCHES_H
