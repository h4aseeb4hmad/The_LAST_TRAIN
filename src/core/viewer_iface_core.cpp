#include "viewer_iface.h"
#include "util.h"

// Terminal UI and dashboard rendering using primitive parameters

#include "level.h"
#include<iostream>
#include<string>
using namespace std;

const int BAR_LENGTH=20;
const string ROOM_NAMES[NUM_FLOORS][ROOMS_PER_FLOOR]={{"Entrance lobby","Reception","Waiting Area","Utility Room"},{"Lounge","Archive","Lab","Meeting Room"},{"Stairwell Annex","Security Vestibule","Animal Room","Control Room"},{"First Aid Station","Medical Room","Maintenance Corridor","Office"},{"Security Room","Train Boarding Platform","Storage Room","Observation Deck"}};

const string CONNECTORS[NUM_FLOORS][ROOMS_PER_FLOOR-1]={{"W","L1","D"},{"D","L1","D"},{"L","W","D"},{"W","#","D"},{"D","D","W"}};
const string OBJECTIVE_NAME[NUM_OBJECTIVES]={"Master Key Collected","Clue digit 4 found","Clue digit 2 found","Clue digit 7 found","Cabinet unlocked (code 427)","Train Key acquired","Fuse collected","Fuse installed at control panel","Lift pass collected","Lift 2 active","Passenger rescued","Cat carrier collected","Cat rescued","Train prepared","Mission won","Mission failed"};

string makeBar(int value,int maxValue) //builds a text bar like [######.....]
{
    value=clampInt(value,0,maxValue);
    int filled=(value*BAR_LENGTH)/maxValue;
    string bar="[";
    for(int i=0;i<BAR_LENGTH;i++)
    {
        if(i<filled)
        {
            bar+="#";
        }
        else
        {
            bar+=".";
        }
    }
    bar+="]";
    return bar;
}

 static bool validRoom(int floor,int room)
{
    if(floor<1 || floor>NUM_FLOORS)
    {
        return false;
    }
    if(room<1 || room>ROOMS_PER_FLOOR)
    {
        return false;
    }
    return true;
}

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score, const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, int emergencyTripsLeft)
{
    printHeader("THE LAST TRAIN - DASHBOARD");
    if(validRoom(playerFloor,playerRoom))
    {
        cout<<"Location : F"<<playerFloor<<"-R"<<playerRoom<<" ("<<ROOM_NAMES[playerFloor-1][playerRoom-1]<<")\n";
    }
    else
    {
        cout<<"Location : Unknown\n";
    }
    cout<<"Energy  : "<<makeBar(energy,MAX_ENERGY)<<" "<<energy<<"/"<<MAX_ENERGY<<"\n";
    cout<<"Time    : "<<makeBar(time,MAX_TIME)<<" "<<time<<"/"<<MAX_TIME<<"\n";
    cout<<"Score   : "<<score<<"\n";
    cout<<"Lift 2 Emergency Trips Left: "<<emergencyTripsLeft<<"\n";
    cout<<"Inventory ("<<inventoryCount<<"/"<<MAX_INVENTORY_CAPACITY<<"): ";
    int safeCount=clampInt(inventoryCount,0,MAX_INVENTORY_CAPACITY); //never read outside the array

    if(safeCount==0)
    {
        cout<<"Empty";
    }
    for(int i=0;i<safeCount;i++)
    {
        cout<<inventory[i]<<" ";
    }
    cout<<"\n";
    for(int i=0;i<safeCount;i++)
    {
        const char*name=getEntityName(inventory[i]);
        cout<<"   "<<inventory[i]<<" = ";
        if(name==nullptr)
        {
            cout<<"Unknown item";
        }
        else
        {
            cout<<name;
        }
        cout<<"\n";
    }
    cout<<"==============================\n";
}

void renderRoomDetails(int floor, int room, const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entityCount)
{
    if(!validRoom(floor,room))
    {
        cout<<"Cannot show details: invalid room.\n";
        return;
    }
    string title="ROOM F"+to_string(floor)+"-R"+to_string(room)+" : "+ROOM_NAMES[floor-1][room-1];
    printHeader(title.c_str());
    int safeCount=clampInt(entityCount,0,MAX_ENTITIES_PER_ROOM);
    if(safeCount==0)
    {
        cout<<"Nothing of interest here.\n";
    }
    else
    {
        cout<<"You can see:\n";
        for(int i=0;i<safeCount;i++)
        {
            char symbol=roomEntities[floor-1][room-1][i];
            const char*name=getEntityName(symbol);
            cout<<"  "<<symbol<<" - ";
            if(name==nullptr)
            {
                cout<<"Unknown object";
            }
            else
            {
                cout<<name;
            }
            cout<<"\n";
        }
    }
    cout<<"==============================\n";
}

void renderAsciiBuilding(int playerFloor, int playerRoom)
{
    printHeader("BUILDING MAP");
    for(int floor=NUM_FLOORS;floor>=1;floor--) //top floor is printed first
    {
        cout<<"Floor "<<floor<<"  ";
        for(int room=1;room<=ROOMS_PER_FLOOR;room++)
        {
            string cell="F"+to_string(floor)+"-R"+to_string(room);
            if(floor==playerFloor && room==playerRoom)
            {
                cout<<"[*"<<cell<<"*]"; //player is in this room
            }
            else
            {
                cout<<"[ "<<cell<<" ]";
            }
            if(room<ROOMS_PER_FLOOR)
            {
                cout<<"-"<<CONNECTORS[floor-1][room-1]<<"-"; //connection to the next room
            }
        }
        cout<<"\n";
    }
    cout<<"\n[*Fx-Rx*] = You are here\n";
    cout<<"D = Door   W = Window   L = Locked Door   # = Solid Wall   L1 = Lift 1\n";
    cout<<"Stairs: S1 (F1-R1 <-> F2-R1), S2(F2-R1 <-> F3-R1), S3 (F4-R1 <-> F5-R1)\n";
    cout<<"Lift 1: F1 <-> F2(R2/R3)   Lift 2: F2-R4, F4-R4, F5-R4\n";
    cout<<"==============================\n";
}

void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES])
{
    printHeader("OBJECTIVES CHECKLIST");
    for(int i=0;i<NUM_OBJECTIVES;i++)
    {
        if(objectives[i])
        {
            cout<<"[X] ";
        }
        else
        {
            cout<<"[ ] ";
        }
        cout<<OBJECTIVE_NAME[i]<<"\n";
    }
    cout<<"==============================\n";
}