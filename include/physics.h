#ifndef PHYSICS_H
#define PHYSICS_H

#include "state.h"

// Movement validation, connectivity & resource adjustments
bool isdirectlyconnected(int fromf, int fromr, int tof, int tor,
                         const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],
                         char& outconntype);

bool cantraverse(int targetfloor, int targetroom, char conntype,
                 const char inventory[MAX_INVENTORY_CAPACITY], int inventorycount,
                 int emergencytripsleft, bool lift2operational);

void applyresourcedelta(int& energy, int& time, int deltaenergy, int deltatime);
void applyscoredelta(int& score, int deltascore);

bool hasitem(const char inventory[MAX_INVENTORY_CAPACITY], int inventorycount, char itemsymbol);
bool additem(char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount, char itemsymbol);
bool removeitem(char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount, char itemsymbol);

#endif // PHYSICS_H
