#include "level.h"
#include "util.h"
#include "physics.h"
#include <fstream>
#include <iostream>
using namespace std;

// Modular implementation using primitive arrays for map parsing and verification

const char* ROOM_NAMES[NUM_FLOORS][ROOMS_PER_FLOOR]=
{
    {"Entrance lobby", "Reception", "Waiting aread", "Utility room"},
    {"Lounge", "Archive", "Lab", "Meeting room"},
    {"Stair well annex", "Security westibule", "Animal room", "Control room"},
    {"First aid station", "Medical room", "Maintenance corridor", "Office"},
    {"Security room", "Train boarding platform", "Storage room", "Observation deck"}

};

const int ITEM_COUNT = 16;
const char ITEM_SYMBOLS[ITEM_COUNT] =
{
    '<', ')' , '\\' , '4', '*', '~', ';', '(', '!', '%', '$', '2', '7', '^', '@', '&'
};

const char* ITEM_NAME[ITEM_COUNT] =
{
    "Master key", "Energy pack", "Emergency toolkit", "Clue digit 4", "Fuse", "Cat", "Control panel", "Lift pass",
     "Injured passenger", "Medical kit", "Emergency access card", "Clue digit 2", "Clue digigt 7", "Cabinet/Train key", "Cat carrier", "Observation token"
};

const char* getentityname(char entitysymbol)
{
    for(int i=0; i < ITEM_COUNT; i++)
    {
        if(ITEM_SYMBOLS[i] == entitysymbol)
            return ITEM_NAME[i];
    }
    return "Unknown";
}

const char* getroomname(int floor, int room)
{
    if(!isvalidroom(floor, room))
        return "Unknown room";

    return ROOM_NAME[floor - 1][room - 1];
}

static bool isknownentity(char symbol)
{
    return !stringequals(getentityname(symbol), "unknown");
}

bool parseroomlabel(const char* text, int& floor, int& room)
{
    if(text == nullptr)
        return false;

    int digits[2];
    int count = 0;
    for(int i=0; text[i] != '\0'; i++)
    {
        char c=text[i];
        if(c >= '0' && c <= '9')
        {
            if(count == 2)
                return false;

            digits[count++] = c - '0';
        }
        else if(c != 'F' && c != 'f' && c != 'r' && c != 'R' && c != '-' && c != ' ')
            return false;

    }
    if(count != 2)
        return false;

    floor = digits[0];
    room = digits[1];
    return true;
}

// Reading file into Dynamic Memory:

static int countlines(const char* filename)
{
    ifstream file(filename);
    if(!file.is_open())
        return -1;
    char buffer[MAX_LINE_LEN];
    int count = 0;
    while(file.getline(bugger, MAX_LINE_LEN))
    {
        count++;
    }
    if(!file.eof())
        return -1;

    return count;
}

static char** readalllines(const char* filename, int linecount)
{
    ifstream file(filename);
    if(!file.is_open() || linecount <= 0)
        return nullptr;

    char** lines = new char*[linecount];
    for(int i=0; i<linecount; i++)
    {
        lines[i] = new char[MAX_LINE_LEN];
        lines[i][0] = '\0';
        file.getline(lines[i], MAX_LINE_LEN);
        int len = stringlength(lines[i]);
        while(len > 0 && (lines[i][len - 1] == '\r' || lines[i][len - 1] == '\n'))
        {
            lines[i][--len] = '\0';
        }
    }
    return lines;
}

static void freealllines(char** lines, int linecount)
{
    for(int i=0; i<linecount; i++)
    {
        delete[] lines[i];
    }
    delete[] lines;
}

const int LABEL_LEN = 5;

static int findlabel(const char* line, int floor, int room)
{
    char label[6] = {'F', char('0' + floor), '-', 'R', char('0' + room), '\0'};
    int length = stringlength(line);
    for(int col=0; col+LABEL_LEN <= length; col++)
    {
        int k=0;
        while(k<LABEL_LEN && line[col + k] == label[k])
        {
            k++;
        }
        if(k == LABEL_LEN)
            return col;
    }
    return -1;
}

static int findfloorline(char** lines, int linecount, int floor, int labelcols[ROOMS_PER_FLOOR])
{
    for(int i=0; i<linecount; i++)
    {
        int r=0;
        while(r<ROOMS_PER_FLOOR)
        {
            labelcols[r] = findlabel(lines[i], floor, r+1)
            if(labelcols[r] == -1 || (r>0 && labelcols[r] < labelcols[r-1]))
                break;

            r++;
        }
        if(r == ROOMS_PER_FLOOR)
            return i;
    }
    return -1;
}

static bool readwalls(const char* line, const int labelcols[ROOMS_PER_FLOOR], char walls[ROOMS_PER_FLOOR], int wallcols[ROOMS_PER_FLOOR])
{
    for(int r=0; r<ROOMS_PER_FLOOR; r++)
    {
        int col= labelcols[r] + LABEL_LEN;
        while(line[col] == ' ')
        {
            col++;
        }
        char symbol = line[col];
        if(symbol == '\0')
            return false;
        if(symbol == 'L' && line[col + 1] == '1')
        {
            symbol = CONN_LIFT1;
            col++;
        }
        if(r == ROOMS_PER_FLOOR - 1)
        {
            if(symbol != '|')
                return false;
            else
                bool goodwall = symbol == CONN_DOOR || CONN_WINDOW || CONN_LIFT1 || CONN_LOCKED_DOOR || CONN_SOLID_WALL;

            if(!goodwall || col >= labelcols[r+1])
                return false;
        }
        walls[r] = symbol;
        wallcols[r] = col;
    }
    return true;
}

static bool readroomitems(const char* itemline, int firstcol, int lastcol, int f, int r, char roomentities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entitycounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    int length = stringlength(itemline);
    for(int col = firstcol; col <= lastcol && col < length; col++)
    {
        char symbol = itemline[col];
        if(symbol == ' ')
            continue;
        if(!isknownentity(symbol))
        {
            cerr << "Map error: Unknown symbol" << symbol << " in F" << (f+1) << "-R" << (r+1) << "\n"; // For errors and warnings.
            return false;
        }

        int copies;
        if(symbol == ENT_ENERGY_PACK)
            copies = ENERGY_PACK_COUNT;
        else
            copies = 1;

        for(int c=0; c<copies; c++)
        {
            if(entitycounts[f][r] >= MAX_ENTITIES_PER_ROOM)
            {
                cerr << "Map error: too many items in F" << (f+1) << "-R" << (r+1) << "\n";
                return false;
            }
            roomentities[f][r][entitycounts[f][r]++] = symbol;
        }

    }
    return true;
}

static bool readfloor(char** lines, int linecount, int floor, char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1], char roomentities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entitycounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    int labelcol[ROOMS_PER_FLOOR];
    int n = findfloorline(lines, linecount, floor, labelcols);
    if(n == -1)
    {
        cerr << "Map error: floor" << floor << " is missing \n";
        return false;
    }
    if(n + 1 >= linecount)
    {
        cerr << "Map error: floor" << floor << " has no item line \n";
        return false;
    }
    char walls[ROOMS_PER_FLOOR];
    int wallcols[ROOMS_PER_FLOOR];
    if(!readwalls(lines[n], labelcols, walls, wallcols))
    {
        cerr << "Map error: bad divider on floor " << floor << "\n";
        return false;
    }

    int f = floor - 1;
    for(int w = 0; w < ROOMS_PER_FLOOR - 1; w++)
    {
        horizontalconn[f][w] = walls[w];
    }

    for(int r=0; r < ROOMS_PER_FLOOR; r++)
    {
        int firstcol;
        if(r==0)
            firstcol = labelcols[0] - 1;
        else
            firstcol = wallcols[r - 1] + 1;

        int lastcol = wallcols[r] - 1;
        if(!readroomitems(lines[n+1], firstcol, lastcol, f, r, roomentities, entitycounts))
            return false;

    }
    return true;
}

bool loadbuildingmap(const char* filename, char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1], char roomentities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entitycounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    for(int f=0; f<NUM_FLOORS; f++)
    {
        for(int w=0; w<ROOMS_PER_FLOOR-1; w++)
        {
            horizontalconn[f][w] = CONN_NONE;
        }
        for(int r=0; r<ROOMS_PER_FLOOR; r++)
        {
            entitycounts[f][r] = 0;
            for(int e=0; e<MAX_ENTITIES_PER_FLOOR; e++)
            {
                roomentities[f][r][e] = ENT_NONE;
            }
        }
    }
    if(filename == nullptr) // reads the file into dynamic memory
    {
        return false;
    }
    int linecount = countlines(filename);
    if(linecount <= 0)
        return false;

    char** lines = readallline(filename, linecount);
    if(lines == nullptr)
    {
        return false;
    }

    // Read 5 floors
    bool ok = true;
    for(int floor = 1; floor <= NUM_FLOORS && ok; floor++)
    {
        ok = readfloor(lines, linecount, floor, horizontalconn, roomentities, entitycounts);
    }
    freealllines(linecount);
    return ok;
}

static bool splitkeyvalue(const char* line, char key[MAX_LINE_LEN], char value[MAX_LINE_LEN])
{
    int colon = 0;
    while(line[colon] != '\0' && line[colon] != ':')
    {
        colon++''
    }

    if(line[colon] != ':')
        return false;

    copystring(key, line, colon+1);
    copystring(value, line+colon+1, MAX_LINE_LEN);
    trimwhitespace(key);
    trimwhitespace(value);
    return key[0] != '\0';
}


static bool readyesno(const char* value, bool& answer)
{
    if(stringequals(value, "YES"))
    {
        answer = true;
        return true;
    }
    if(stringequals(value, "NO"))
    {
        answer = false;
        return true;
    }
    return false;
}

static bool checkmaximum(const char* key, const char* value, int gamemaximum)
{
    int filemaximum = 0;
    if(!parseint(value, filemaximum))
        return false;
    if(filemaximum != gamemaximum)
    {
        cerr << "Warning: " << key << " in the state file is " << filemaximum << " but the game uses " << gamemaximum << "\n"; 
    }
    return true;
}

static bool readclues(const char* value, bool objectives[NUM_OBJECTIVES])
{
    if(stringequals(value, "NONE"))
        return true;
    for(int i=0; value[i] != '\0'; i++)
    {
        char c = value[i];
        if(c == ENT_CLUE_4)
            objectives[OBJ_CLUE_4] = true;
        else if(c == ENT_CLUE_2)
            objectives[OBJ_CLUE_2] = true;
        else if(c == ENT_CLUE_7)
            objectives[OBJ_CLUE_7] = true;
        else if(c != ',' && c != ' ')
            return false;
    }
    return true  ;
}

static bool readinventory(const char* value , char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount){
    if stringequals(value , "NONE ")
        return true;
    for(int i = 0; value[i] != '\0'; i++){
        char c = value[i];
        if(c == ' '|| c== ',')
            continue;
    if(!isknownentity(c)|| !additem(inventory, inventorycount, c ))
        return false;
    }
    return true;
}

static bool usesetting(const char* key, const char* value, int& playerfloor, int* playerroom, int* energy, int* time, int* score, char inventory[MAX_INVENTORY_CAPACITY], int* inventorycount, bool objectives[NUM_OBJECTIVES])
{
    bool notused = false;
    if(stringequals(key, "GAME_STATUS"))
        return true;
    if(stringequals(key, "START_ROOM"))
    {
        if(stringequals(value, "PLAYER_SELECTS"))
        {
            playerfloor = 0; // 0 means ask the player
            playerroom = 0; 
            return true;
        }
        return parseroomlabel(value, playerfloor, playerroom) && isvalidroom(playerfloor, playerroom);
    }
    if(stringequals(key, "INITIAL_ENERGY"))
        return parseint(value, energy);
    if(stringequals(key, "INITIAL_TIME"))
        return parseint(value, time);
    if(stringequals(key, "INITIAL_SCORE"))
        return parseint(value, score);
    if(stringequals(key, "MAX_ENERGY"))
        return checkmaximum(key, value, MAX_ENERGY);
    if(stringequals(key, "MAX_TIME"))
        return checkmaximum(key, value, MAX_TIME);
    if(stringequals(key, "INITIAL_INVENTORY"))
        return readinventory(value, inventory, inventorycount);
    if(stringequals(key, "CLUE_COLLECTED"))
        return readclues(value, objectives);
    if(stringequals(key, "PASSENGER_RESCUED"))
        return readyesno(value, objectives[OBJ_PASSENGER_RESCUED]);
    if(stringequals(key, "CAT_RESCUED"))
        return readyesno(value, objectives[OBJ_CAT_RESCUED]);
    if(stringequals(key, "CABINET_UNLOCKED"))
        return readyesno(value, objectives[OBJ_CABINET_UNLOCKED]);
    if(stringequals(key, "TRAIN_KEY_COLLECTED"))
        return readyesno(value, objectives[OBJ_TRAIN_KEY]);
    if(stringequals(key, "FUSE_INSTALLED"))
        return readyesno(value, objectives[OBJ_FUSE_INSTALLED]);
    if(stringequals(key, "LIFT_PASS_COLLECTED"))
        return readyesno(value, objectives[OBJ_LIFT_PASS]);
    if(stringequals(key, "TRAIN_PREPARED"))
        return readyesno(value, objectives[OBJ_TRAIN_PREPARED]);
    if(stringequals(key, "PLAYER_AT_TRAIN"))
        return readyesno(value, notused);
    
    cerr << "Warning: Unknown setting " << key << " ignored \n";
    return true;

}

bool loadinitialstate(const char* filename, int& playerfloor, int& playerroom, int& energy, int& time, int& score, char inventory[MAX_INVENTORY_CAPACITY], int& inventorycount, bool objectives[NUM_OBJECTIVES], int& emergencytripsleft)
{
    playerfloor = 0;
    playerroom = 0;
    energy = INITIAL_ENERGY;
    time = INITIAL_TIME;
    score = INITIAL_SCORE;
    inventorycount = 0;
    emergencytripsleft = 0;
    for(int i = 0; i<MAX_INVENTORY_CAPACITY; i++)
    {
        inventory[i] = ENT_NONE;
    }
    for(int i = 0; i<NUM_OBJECTIVES; i++)
    {
        objectives[i] = false;
    }

    // read file line by line
    if(filename == nullptr)
        return false;

    ifstream file(filename);
    if(!file.is_open())
        return false;

    char line[MAX_LINE_LEN], key[MAX_LINE_LEN], value[MAX_LINE_LEN];
    int linenumber = 0;
    while(file.getline(line, MAX_LINE_LEN))
    {
        linenumber++;
        trimwhitespace(line);
        if(line[0] == '\0')
            continue;
        if(!splitkeyvalue(line, key, value) || !usesetting(key, value, playerfloor, playerroom, energy, time, score, inventory, inventorycount, objectives))
        {
            c
            err << "State file error on line " << linenumber << ": " << line << "\n";
            return false;
        }
    }
    if(!file.eof())
    {
        cerr << "State file error on line " << linenumber + 1 << " is too long\n";
            return false;
    }

    energy = clampint(energy, 0, MAX_ENERGY);
    time = clampint(time, 0, MAX_TIME);
    score = clampint(score, 0, MAX_SCORE);
    if(hasitem(inventory, inventorycount, ENT_EMERGENCY_CARD))
    {
        emergencytripsleft = EMERGENCY_TRIPS_GRANTED;
    }
    return true;
}
static int markreachablerooms (int startfloor , int startroom , const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1 ], bool lockeddooropen , bool lift2open, bool visited[NUM_FLOORS][ROOMS_PER_FLOOR]){
for(int f = 0 ; f<NUM_FLOORS ; f++)
    for(int r = 0 ; r < ROOMS_PER_FLOOR; r++ ){
        visited[f][r] = false;

    }
    if(!isvalidroom(startfloor , startroom))
    return 0;

    visited[startfloor -1 ][startroom - 1 ] = true ;
    int count = 1 ;
    bool foundnew = true;
    while(foundnew ){
        foundnew = false;
        for (int fromf = 1 ; fromf <= NUM_FLOORS; fromf ++)
        for(int fromr = 1 ; fromr <= ROOMS_PER_FLOOR; fromr++){
            if(!visited[fromf - 1][fromr -1])
                continue;
        for(int tof = 1 ; tof <= NUM_FLOORS; tof ++)
        for(int tor = 1 ; tor<= ROOMS_PER_FLOOR; tor ++){
            char way = CONN_NONE;
            if(visited[tof -1 ][tor - 1])
            continue;
            if(!isdirectlyconnected(fromf , fromr , tof , tor , horizontalconn , way))
            continue;
            if(way == CONN_LOCKED_DOOR && !lockeddooropen)
            continue;
            if(way == CONN_LIFT2 && !lift2open)
            continue;
            visited[tof -1 ][tor - 1 ] = true ;
            count ++;
            foundnew = true;

        }
     }
        
    }
    return count;
}
int countreachablerooms(int startfloor , int startroom , const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1 ]){
    bool visited[NUM_FLOORS][ROOMS_PER_FLOOR];
    return markreachablerooms(startfloor , startroom, horizontalconn , true , true , visited);
}
bool validatestartingroom(int startfloor , int startroom , const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR -1 ]){
    if(!isvalidroom(startfloor , startroom))
    return false;
return countreachablerooms(startfloor , startroom , horizontalconn) == TOTAL_ROOMS;
}
static bool iteminmarkedroom(char symbol , const bool visited[NUM_FLOORS][ROOMS_PER_FLOOR], const char roomentities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], const int entitycounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
    for(int f = 0 ; f < NUM_FLOORS ; f++)
    for(int r = 0; r < ROOMS_PER_FLOOR; r++)
for(int e = 0 ; visited[f][r] && e < entitycounts[f][r];e++)
if(roomentities[f][r][e]== symbol )
return true;

return false;    
}
bool isstartroomplayable(int startfloor , int startroom ,const char horizontalconn[NUM_FLOORS][ROOMS_PER_FLOOR - 1], const char roomentities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], const int entitycounts[NUM_FLOORS][ROOMS_PER_FLOOR]){
    bool visited[NUM_FLOORS][ROOMS_PER_FLOOR];
    bool dooropen = false , liftopen = false , somethingopened = true;
int count = 0;
while(somethingopened){
    somethingopened = false ;
    count = markreachablerooms(startfloor , startroom , horizontalconn, dooropen , liftopen ,visited){
        if(!dooropen && iteminmarkedroom(ENT_MASTER_KEY , visited , roomentities , entitycounts))
        dooropen = somethingopened = true ;
    bool card = iteminmarkedroom(ENT_EMERGENCY_CARD , visited , roomentities , entitycounts);
    bool repair = iteminmarkedroom(ENT_FUSE , visited , roomentities , entitycounts) && iteminmarkedroom(ENT_CONTROL_PANEL, visited , roomentities , entitycounts) && iteminmarkedroom(ENT_LIFT_PASS, visited , roomentities , entitycounts);
if(!liftopen && (card || repair))
liftopen = somethingopened = true ;

    }
}
return count == TOTAL_ROOMS;
}