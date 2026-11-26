#include "viewer_iface.h"
#include "util.h"

// Terminal UI and dashboard rendering using primitive parameters

#include "level.h"
#include<iostream>
#include<string>
using namespace std;

const int BAR_LENGTH=20;
const string ROOM_NAMES[NUM_FLOORS][ROOMS_PER_FLOOR]={{"Entrance lobby","Reception","Waiting Area","Utility Room"},{"Lounge","Archive","Lab","Meeting Room"},{"Stairwell Annex","Security Vestibule","Animal Room","Control Room"},{"First Aid Station","Medical Room","Maintenance Corridor","Office"},{"Security Room","Train Boarding Platform","Storage Room","Observation Deck"}};

const string CONNECTORS[NUM_FLOORS][ROOMS_PER_FLOOR-1]={{"W","L1","D"},{"D","L1","D"},{"L","W","D"},{"W","#","D"},{"D","D","W"}}; //this array stores connection between rooms as there are 4 rooms so there is 3 connections between them
const string OBJECTIVE_NAME[NUM_OBJECTIVES]={"Master Key Collected","Clue digit 4 found","Clue digit 2 found","Clue digit 7 found","Cabinet unlocked (code 427)","Train Key acquired","Fuse collected","Fuse installed at control panel","Lift pass collected","Lift 2 active","Passenger rescued","Cat carrier collected","Cat rescued","Train prepared","Mission won","Mission failed"}; //this array stores objectives of the game.each position represent one objective

string makeBar(int value,int maxValue) //builds a text bar like [######.....]  //creates energy and time bars
{
    value=clampInt(value,0,maxValue); //make sure value stays between 0 and max value
    int filled=(value*BAR_LENGTH)/maxValue; //this calculates how many positions of the bar should contain #
    string bar="["; //created empty string and put an opening bracket inside it
    for(int i=0;i<BAR_LENGTH;i++) //runs 20 times as bar length is 20
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

void renderDashboard(int playerFloor, int playerRoom, int energy, int time, int score, const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, int emergencyTripsLeft) //displays player information
{
    printHeader("THE LAST TRAIN - DASHBOARD");
    if(validRoom(playerFloor,playerRoom)) //checking player information as location: F2-R3 (Lab)
    {
        cout<<"Location : F"<<playerFloor<<"-R"<<playerRoom<<" ("<<ROOM_NAMES[playerFloor-1][playerRoom-1]<<")\n"; //-1 because array indexes start from 0 but our floor and room number start from 1 so thats why
    }
    else
    {
        cout<<"Location : Unknown\n";
    }
    cout<<"Energy  : "<<makeBar(energy,MAX_ENERGY)<<" "<<energy<<"/"<<MAX_ENERGY<<"\n"; //displays energy of player
    cout<<"Time    : "<<makeBar(time,MAX_TIME)<<" "<<time<<"/"<<MAX_TIME<<"\n"; //displays remaining game time
    cout<<"Score   : "<<score<<"\n"; //displays current score of player
    cout<<"Lift 2 Emergency Trips Left: "<<emergencyTripsLeft<<"\n"; //displays how many emergency trips are still available for lift 2
    cout<<"Inventory ("<<inventoryCount<<"/"<<MAX_INVENTORY_CAPACITY<<"): "; //displays number of collected item and maximum inventory capacity 
    int safeCount=clampInt(inventoryCount,0,MAX_INVENTORY_CAPACITY); //never read outside the array if inventory count becomes 15 and maximum is 10 we could access memory outside array

    if(safeCount==0)
    {
        cout<<"Empty";
    }
    for(int i=0;i<safeCount;i++) //displays all the item symbol stored in inventory array
    {
        cout<<inventory[i]<<" ";
    }
    cout<<"\n";
    for(int i=0;i<safeCount;i++) //displays full name of inventory items
    {
        const char*name=getEntityName(inventory[i]);
        cout<<"   "<<inventory[i]<<" = "; //K = Master Key, F = Fuse
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

void renderRoomDetails(int floor, int room, const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entityCount) //displays objects inside a room
{
    if(!validRoom(floor,room))
    {
        cout<<"Cannot show details: invalid room.\n";
        return;
    }
    string title="ROOM F"+to_string(floor)+"-R"+to_string(room)+" : "+ROOM_NAMES[floor-1][room-1]; //to string converts integer to string as floor=2;room=3 becomes ROOM F2-R3 : Lab
    printHeader(title.c_str()); //.c_str converts into a form that can be passed as const char* because title is c++ while print header is given c-style string
    int safeCount=clampInt(entityCount,0,MAX_ENTITIES_PER_ROOM);
    if(safeCount==0)
    {
        cout<<"Nothing of interest here.\n";
    }
    else //dispaying objects in the room
    {
        cout<<"You can see:\n";
        for(int i=0;i<safeCount;i++)
        {
            char symbol=roomEntities[floor-1][room-1][i];
            const char*name=getEntityName(symbol); //finds full name of object using its symbol
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

void renderAsciiBuilding(int playerFloor, int playerRoom) //displays the building map all 5 floors and their 4 rooms it also shows where the player currently is and how rooms are connected
{
    printHeader("BUILDING MAP");
    for(int floor=NUM_FLOORS;floor>=1;floor--) //top floor is printed first and the ground floor last so floor--
    {
        cout<<"Floor "<<floor<<"  ";
        for(int room=1;room<=ROOMS_PER_FLOOR;room++)
        {
            string cell="F"+to_string(floor)+"-R"+to_string(room); //using to_string because floor and room are integers but we want to combine them with text
            if(floor==playerFloor && room==playerRoom) //checks if current room is the players actual location
            {
                cout<<"[*"<<cell<<"*]"; //player is in this room as indicated by *
            }
            else
            {
                cout<<"[ "<<cell<<" ]"; //displays room normally 
            }
            if(room<ROOMS_PER_FLOOR) //displays connection between current room and next room
            {
                cout<<"-"<<CONNECTORS[floor-1][room-1]<<"-"; //[ F1-R1 ]-W-[ F1-R2]
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

void renderObjectivesChecklist(const bool objectives[NUM_OBJECTIVES]) //displays completed and incomplete objectives
{
    printHeader("OBJECTIVES CHECKLIST");
    for(int i=0;i<NUM_OBJECTIVES;i++)
    {
        if(objectives[i]) //this checks bolean value true or false if true then displays x as below means objective is completed
        {
            cout<<"[X] ";
        }
        else
        {
            cout<<"[ ] ";
        }
        cout<<OBJECTIVE_NAME[i]<<"\n"; //displays name of objective as Master Key Collected
    }
    cout<<"==============================\n";
}