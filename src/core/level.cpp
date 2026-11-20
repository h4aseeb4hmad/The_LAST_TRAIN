#include "level.h"
#include "util.h"

// Modular implementation using primitive arrays for map parsing and verification
#include<iostream>
#include<fstream>
#include<cstring>
#include<cstdlib>
using namespace std;

const char CONN_LIFT_1='1';
const int MAX_MAP_LINES=60;

static int find_label_column(const char*line, int floorNumber, int roomNumber)
{
    char label[20];
    label[0]='F';
    label[1]='0'+floorNumber;
    label[2]='-';
    label[3]='R';
    label[4]='0'+roomNumber;
    label[5]='\0';
    const char*found=strstr(line, label);
    if(found==nullptr)
    {
        return -1;
    }
    return (int)(found-line);
}

static char read_connector(const char*line,int left_label_column, int right_label_column)
{
    char found[5];
    int count=0;
    for(int i=left_label_column+5;i<right_label_column;i++)
    {
        if(line[i]!=' ' && count<4)
        {
            found[count]=line[i];
            count++;
        }
    }
    found[count]='\0';
    if(stringEquals(found,"D"))
    {
        return CONN_DOOR;
    }
     if(stringEquals(found,"W"))
    {
        return CONN_WINDOW;
    }
     if(stringEquals(found,"L"))
    {
        return CONN_LOCKED_DOOR;
    }
     if(stringEquals(found,"#"))
    {
        return CONN_SOLID_WALL;
    }
     if(stringEquals(found,"L1"))
    {
        return CONN_LIFT_1;
    }
    return CONN_NONE;
}

static bool entity_symbol(char c)
{
    if(c==ENT_NONE)
    {
        return false;
    }
    return !stringEquals(getEntityName(c), "Unknown");
}

bool loadBuildingMap(const char* filename, char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1], char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
    for(int f=0;f<NUM_FLOORS;f++)
    {
        for(int c=0;c<ROOMS_PER_FLOOR-1;c++)
        {
            horizontalConn[f][c]=CONN_NONE;
        }
        for(int r=0;r<ROOMS_PER_FLOOR;r++)
        {
            entityCounts[f][r]=0;
            for(int e=0;e<MAX_ENTITIES_PER_ROOM;e++)
            {
                roomEntities[f][r][e]=ENT_NONE;
         }
    }
}

ifstream file(filename);
if(!file.is_open())
{
    return false;
}

char lines[MAX_MAP_LINES][MAX_LINE_LEN];
int line_count=0;
while(line_count<MAX_MAP_LINES && file.getline(lines[line_count], MAX_LINE_LEN))
{
    int len=(int)strlen(lines[line_count]);
    while(len>0 && (lines[line_count][len-1]=='\n' || lines[line_count][len-1]=='\r'))
    {
        lines[line_count][len-1]='\0';
        len--;
    }
    line_count++;
}
file.close();

for(int floor_number=1;floor_number<=NUM_FLOORS;floor_number++)
{
    int f=floor_number-1;
    bool floor_found=false;
    for(int i=0;i<line_count-1;i++)
    {
        int col[ROOMS_PER_FLOOR];
        bool all_found=true;
        for(int r=0;r<ROOMS_PER_FLOOR;r++)
        {
            col[r]=find_label_column(lines[i], floor_number, r+1);
            if(col[r]==-1)
            {
                all_found=false;
                break;
            }
        }
        if(!all_found)
        {
            continue;
        }
        floor_found=true;
        for(int c=0;c<ROOMS_PER_FLOOR-1;c++)
        {
            horizontalConn[f][c]=read_connector(lines[i], col[c], col[c+1]);
        }

        const char*item_line=lines[i+1];
        int item_line_len=(int)strlen(item_line);
        for(int r=0;r<ROOMS_PER_FLOOR;r++)
        {
            int start=col[r]-1;
            if(start<0)
            {
                start=0;
            }
            int end=start+11;
            for(int x=start;x<end && x<item_line_len;x++)
            {
                if(entity_symbol(item_line[x]))
                {
                    if(entityCounts[f][r]>=MAX_ENTITIES_PER_ROOM)
                    {
                        return false;
                    }
                    roomEntities[f][r][entityCounts[f][r]]=item_line[x];
                    entityCounts[f][r]++;
                }
            }
        }
        break;
    }
    if(!floor_found)
    {
        return false;
    }
}
return true;
}

bool loadInitialState(const char* filename, int& playerFloor, int& playerRoom, int& energy, int& time, int& score, char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft)
// how the game will start
{
    (void)playerFloor;
    (void)playerRoom;
    ifstream file(filename);
    if(!file.is_open())
    {
        return false;
    }
    energy=INITIAL_ENERGY;
    time=INITIAL_TIME;
    score=INITIAL_SCORE;
    inventoryCount=0;
    emergencyTripsLeft=0;
    for(int i=0;i<MAX_INVENTORY_CAPACITY;i++)
    {
        inventory[i]=ENT_NONE;
    }
    for(int i=0;i<NUM_OBJECTIVES;i++)
    {
        objectives[i]=false;
    }
    char line[MAX_LINE_LEN];
    char key[MAX_LINE_LEN];
    char value[MAX_LINE_LEN];
    while(file.getline(line, MAX_LINE_LEN))
    {
        char*colon=strchr(line,':');   // searches for the colon
        if(colon==nullptr)
        {
            continue;
        }
        int key_length=(int)(colon-line);
        strncpy(key,line,key_length);
        key[key_length]='\0';
        strncpy(value,colon+1,MAX_LINE_LEN-1);
        value[MAX_LINE_LEN-1]='\0';

        trimWhitespace(key);
        trimWhitespace(value);
        if(stringEquals(key,"INITIAL_ENERGY"))
        {
            energy=atoi(value);
        }
        else if(stringEquals(key,"INITIAL_TIME"))
        {
            time=atoi(value);
        }
         else if(stringEquals(key,"INITIAL_SCORE"))
        {
            score=atoi(value);
        }
         else if(stringEquals(key,"INITIAL_INVENTORY"))
        {
            if(!stringEquals(value,"NONE"))
            {
                for(int i=0;value[i]!='\0';i++)
                {
                    if(entity_symbol(value[i]) && inventoryCount<MAX_INVENTORY_CAPACITY)
                    {
                        inventory[inventoryCount]=value[i];
                        inventoryCount++;
                    }
                }
            }
        }
        else if(stringEquals(key,"CLUES_COLLECTED"))
        {
            for(int i=0;value[i]!='\0';i++)
            {
                if(value[i]==ENT_CLUE_4)
                {
                    objectives[OBJ_CLUE_4]=true;
                }
                if(value[i]==ENT_CLUE_2)
                {
                    objectives[OBJ_CLUE_2]=true;
                }
                if(value[i]==ENT_CLUE_7)
                {
                    objectives[OBJ_CLUE_7]=true;
                }
            }
        }
            else if(stringEquals(key,"PASSENGER_RESCUED")) // checking if game missions are completed or not
            {
                objectives[OBJ_PASSENGER_RESCUED]=stringEquals(value,"YES");
            }
             else if(stringEquals(key,"CAT_RESCUED"))
            {
                objectives[OBJ_CAT_SECURED]=stringEquals(value,"YES");
            }
             else if(stringEquals(key,"CABINET_UNLOCKED"))
            {
                objectives[OBJ_CABINET_UNLOCKED]=stringEquals(value,"YES");
            }
             else if(stringEquals(key,"TRAIN_KEY_COLLECTED"))
            {
                objectives[OBJ_TRAIN_KEY]=stringEquals(value,"YES");
            }
             else if(stringEquals(key,"FUSE_INSTALLED"))
            {
                objectives[OBJ_FUSE_INSTALLED]=stringEquals(value,"YES");
            }
             else if(stringEquals(key,"LIFT_PASS_COLLECTED"))
            {
                objectives[OBJ_LIFT_PASS]=stringEquals(value,"YES");
            }
             else if(stringEquals(key,"TRAIN_PREPARED"))
            {
                objectives[OBJ_TRAIN_PREPARED]=stringEquals(value,"YES");
            }
        
        }
        file.close();
        energy=clampInt(energy,0,MAX_ENERGY);  // clampint keeps number in range eg 150 energy converts to 100
        time=clampInt(time,0,MAX_TIME);
        return true;

    }

    bool validateStartingRoom(int startFloor, int startRoom, const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1])
    {
        if(startFloor<1 || startFloor>NUM_FLOORS)
        {
            return false;
        }
        if(startRoom<1 || startRoom>ROOMS_PER_FLOOR)
        {
            return false;
        }
        int f=startFloor-1; // -1 because indexes start from 0 but floor numbers for 1 so
        if(startRoom>1) // checking left connection if player can leave room from the left side
        {
            char left=horizontalConn[f][startRoom-2]; 
            if(left!=CONN_NONE && left!=CONN_SOLID_WALL)
            {
                return true;
            }
        }
        if(startRoom<ROOMS_PER_FLOOR) // checking right connection
        {
            char right=horizontalConn[f][startRoom-1];
            if(right!=CONN_NONE && right!=CONN_SOLID_WALL)
            {
                return true;
            }
        }
        if(startRoom==1) 
        {
            return true;
        }
        if((startFloor==1 || startFloor==2) && (startRoom==2 || startRoom==3)) // checking rooms served by lift 1
        {
            return true;
        }
        if(startRoom==4 && (startFloor==2 || startFloor==4 || startFloor==5)) // checking rooms served by lift 2
        {
            return true;
        }
        return false; // player is trapped
    }

    const char*getEntityName(char entitySymbol)
    {
        if(entitySymbol==ENT_NONE)
        {
            return "Empty";
        }
         if(entitySymbol==ENT_MASTER_KEY)
        {
            return "Master Key";
        }
         if(entitySymbol==ENT_ENERGY_PACK)
        {
            return "Energy Pack";
        }
         if(entitySymbol==ENT_EMERGENCY_TOOLKIT)
        {
            return "Emergency Toolkit";
        }
         if(entitySymbol==ENT_CLUE_4)
        {
            return "Clue Digit 4";
        }
         if(entitySymbol==ENT_FUSE)
        {
            return "Fuse";
        }
         if(entitySymbol==ENT_CAT)
        {
            return "Cat";
        }
         if(entitySymbol==ENT_CONTROL_PANEL)
        {
            return "Control Panel";
        }
         if(entitySymbol==ENT_LIFT_PASS)
        {
            return "Lift Pass";
        }
         if(entitySymbol==ENT_PASSENGER)
        {
            return "Injured Passenger";
        }
         if(entitySymbol==ENT_MEDICAL_KIT)
        {
            return "Medical Kit";
        }
         if(entitySymbol==ENT_EMERGENCY_CARD)
        {
            return "Emergency Access Card";
        }
         if(entitySymbol==ENT_CLUE_2)
        {
            return "Clue Digit 2";
        }
         if(entitySymbol==ENT_CLUE_7)
        {
            return "Clue Digit 7";
        }
         if(entitySymbol==ENT_CABINET_TRAIN_KEY)
        {
            return "Cabinet/Train Key";
        }
         if(entitySymbol==ENT_CAT_CARRIER)
        {
            return "Cat Carrier";
        }
         if(entitySymbol==ENT_OBSERVATION_TOKEN)
        {
            return "Observation Token";
        }
        return "Unknown";
    }

