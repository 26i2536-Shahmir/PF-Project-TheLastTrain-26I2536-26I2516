#include "physics.h"
#include "switches.h"
#include "util.h"


// Movement validation and resource adjustments without structures

bool isvalidroom(int floor, int room)
{
    if(floor < 1 || floor > NUM_FLOORS)
        return false;
    if(room < 1 || room > ROOMS_PER_FLOOR)
        return false;

    return true;
}

static bool isstairlink(int fromf, int fromr, int tof, int tor)
{
    if(fromr != 1 || tor != 1)
        return false;
    if((fromf == 1 && tof == 2) || (fromf == 2 && tof == 1))
        return true;
    if((fromf == 2 && tof == 3) || (fromf == 3 && tof == 2))
        return true;
    if((fromf == 4 && tof == 5) || (fromf == 5 && tof == 4))
        return true;

    return false;
}

static bool isbesidelift1(int floor, int room, const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
{
    int f = floor - 1;
    if(room > 1 && horizontalconn[f][room-2] == CONN_LIFT1)
        return true;
    if(room < ROOMS_PER_FLOOR && horizontalconn[f][room-1] == CONN_LIFT1)
        return true;

    return false;
}

bool isdirectlyconnected(int fromf, int fromr, int tof, int tor, const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1], char& conntype)
{
    conntype = CONN_NONE;
    if(!isvalidroom(fromf,fromr) || !isvalidroom(tof,tor))
        return false;
    if(fromf == tof && fromr == tor)
        return false;
    if(fromf == tof)
    {
        if(fromr - tor != 1 && tor - fromr != 1)
            return false;
        int leftroom = fromr;
        if(tor < fromr)
            leftroom = tor;

        char wall = horizontalconn[fromf - 1][leftroom - 1];

        if(wall == CONN_DOOR || wall == CONN_WINDOW || wall == CONN_LOCKED_DOOR)
        {
            conntype = wall;
            return true;
        }
        if(wall == CONN_SOLID_WALL)
            conntype = CONN_SOLID_WALL;

        return false;
    }

    if(isstairlink(fromf, fromr, tof, tor))
    {
        conntype = CONN_STAIRS;
        return true;
    }

    if(canuselift1(fromf, tof) && isbesidelift1(fromf, fromr, horizontalconn) && isbesidelift1(tof, tor, horizontalconn))
    {
        conntype = CONN_LIFT1;
        return true;
    }

    if(fromr == 4 && tor == 4 && islift2floor(fromf) && islift2floor(tof))
    {
        conntype = CONN_LIFT2;
        return true;
    }

    return false;
}

bool cantraverse(int targetfloor, int targetroom, char conntype, const char inventory[MAX_INVENTORY_CAPACITY], int inventorycount, int emergencytripsleft, bool lift2operational)
{
    if(!isvalidroom(targetfloor, targetroom))
        return false;
    if(conntype == CONN_DOOR || conntype == CONN_WINDOW || conntype == CONN_STAIRS || conntype == CONN_LIFT1)
        return true;
    if(conntype == CONN_LOCKED_DOOR)
        return unlockfloor3door(inventory, inventorycount);
    if(conntype == CONN_LIFT2)
    {
        if(lift2operational || emergencytripsleft > 0)
            return true;

        return false;
    }
    return false;
}

void applyresourcedelta(int& energy, int& time, int deltaenergy, int deltatime)
{
    energy = clampint(energy + deltaenergy, 0, MAX_ENERGY);
    time = clampint(time + deltatime, 0, MAX_TIME);
}

void applyscoredelta(int& score, int deltascore)
{
    score = clampint(score + deltascore, 0, MAX_SCORE);
}

bool hasitem(const char inventory[MAX_INVENTORY_CAPACITY], int inventorycount, char itemsymbol)
{
    if(inventorycount < 0 || inventorycount > MAX_INVENTORY_CAPACITY)
        return false;

    for(int i=0; i < inventorycount; i++)
    {
        if(inventory[i] == itemsymbol)
            return true;
    }
    return false;
}

bool additem(char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount, char itemsymbol)
{
    if(inventorycount < 0 || inventorycount >= MAX_INVENTORY_CAPACITY)
        return false;
    
    inventory[inventorycount] = itemsymbol;
    inventorycount++;
    return true;
}

bool removeitem(char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount, char itemsymbol)
{
    if(inventorycount <= 0 || inventorycount > MAX_INVENTORY_CAPACITY)
        return false;

    int position = -1;
    for(int i=0; i < inventorycount; i++)
    {
        if(inventory[i] == itemsymbol)
        {
            position = i;
            break;
        }
    }
    if(position == -1)
        return false;

    for(int i = position; i < inventorycount - 1; i++)
    {
        inventory[i] = inventory[i+1];
    }

    inventorycount--;
    inventory[inventorycount] = ENT_NONE;
    return true;
}