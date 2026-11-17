#include "switches.h"
#include "physics.h"
#include "util.h"

// Door lock mechanics, lifts, and cabinet verification without structures
bool islift2floor(int floor)
{
    if(floor == 2 || floor == 4 || floor == 5)
        return true;

    return false;
}

bool canuselift1(int fromfloor, int tofloor)
{
    if(fromfloor == 1 && tofloor == 2)
        return true;
    if(fromfloor == 2 && tofloor == 1)
        return true;

    return false;
}

bool canuselift2(int fromfloor, int tofloor, bool lift2operational, int emergencytripsleft)
{
    if(!islift2floor(fromfloor) || !islift2floor(tofloor))
        return false;
    if(fromfloor == tofloor)
        return false;
    if(lift2operational)
        return true;
    if(emergencytripsleft > 0)
        return true;

    return false;
}

bool unlockfloor3door(const char inventory[MAX_INVENTORY_CAPACITY], int inventorycount)
{
    return hasitem(inventory, inventorycount, ENT_MASTER_KEY);
}

bool installfusecontrolpanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount, bool objectives[NUM_OBJECTIVES], int& score)
{
    if(objectives[OBJ_FUSE_INSTALLED])
        return false;
    if(!hasitem(inventory, inventorycount, ENT_FUSE))
        return false;
    
    removeitem(inventory, inventorycount, ENT_FUSE);
    objectives[OBJ_FUSE_INSTALLED] = true;
    applyscoredelta(score, SCORE_INSTALL_FUSE);
    return true;
}

bool verifycabinetcode(const char* enteredcode, const bool objectives[NUM_OBJECTIVES])
{
    if(!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7] )
        return false;

    return stringequals(enteredcode, "427");
}

