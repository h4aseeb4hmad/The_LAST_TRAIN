#include "physics.h"
#include "util.h"

// Movement validation and resource adjustments without structures
#include<iostream>
using namespace std;

const char STAIR_TYPE='S';
const char LIFT1_TYPE='1';
const char LIFT2_TYPE='2';
const int MAX_SCORE=520;

bool validRoom(int floor, int room)
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

bool isDirectlyConnected(int fromF, int fromR, int toF, int toR, const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1], char& outConnType) //checking if two rooms are linked and finding how they are linked
{
    outConnType=CONN_NONE;
    if(!validRoom(fromF,fromR) || !validRoom(toF,toR))
    {
        return false;
    }
    if(fromF==toF && fromR==toR)
    {
        return false;
    }
    if(fromF==toF)
    {
        int difference=toR-fromR;
        if(difference!=1 && difference!=-1)
        {
            return false;
        }
        int leftRoom=fromR;
        if(toR<fromR)
        {
            leftRoom=toR;
        }
        char conn=horizontalConn[fromF-1][leftRoom-1];
        outConnType=conn;
        if(conn==CONN_NONE || conn==CONN_SOLID_WALL)
        {
            return false;
        }
        return true;
    }
    if(fromR==1 && toR==1) //connection between different floors
    {
        int lowFloor=fromF;
        if(toF<fromF)
        {
            lowFloor=toF;
        }
    int highFloor=fromF+toF-lowFloor;
    if(lowFloor==1 && highFloor==2) //checking which  floor have stairs
    {
        outConnType=STAIR_TYPE;
        return true;
    }
    if(lowFloor==2 && highFloor==3)
    {
        outConnType=STAIR_TYPE;
        return true;
    }
    if(lowFloor==4 && highFloor==5)
    {
        outConnType=STAIR_TYPE;
        return true;
    }
}

bool fromLift1room=(fromR==2 || fromR==3);
bool toLift1room=(toR==2 || toR==3);
if(((fromF==1 && toF==2) || (fromF==2 && toF==1)) && fromLift1room && toLift1room)
{
    outConnType=LIFT1_TYPE;
    return true;
}
bool fromLift2floor=(fromF==2 || fromF==4 || fromF==5);
bool toLift2floor=(toF==2 || toF==4 || toF==5);
if(fromR==4 && toR==4 && fromLift2floor && toLift2floor)
{
    outConnType=LIFT2_TYPE;
    return true;
}
return false;
}

bool canTraverse(int targetFloor, int targetRoom, char connType, const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, int emergencyTripsLeft, bool lift2Operational) //checking if the player is allowed to pass through a connection
{
    if(!validRoom(targetFloor,targetRoom))
    {
        return false;
    }
    if(connType==CONN_DOOR || connType==CONN_WINDOW || connType==STAIR_TYPE)
    {
        return true; //they are always open
    }
    if(connType==CONN_LOCKED_DOOR)
    {
        return hasItem(inventory,inventoryCount,ENT_MASTER_KEY); //locked door needs master key
    }
    if(connType==LIFT1_TYPE)
    {
        return true; //lift 1 always accesible
    }
    if(connType==LIFT2_TYPE)
    {
        if(targetRoom!=4) //lift 2 can only take player to room 4
        {
            return false;
        }
        if(targetFloor!=2 && targetFloor!=4 && targetFloor!=5)
        {
            return false;
        }
        if(lift2Operational) //fuse installed and lift pass collected
        {
            return true;
        }
        return emergencyTripsLeft>0; //when lift2 is not working we can use emergencytrips if available
    }
    return false; //solid wall no connection or an unknown type
}

void applyResourceDelta(int& energy, int& time, int deltaEnergy, int deltaTime) //updates player energy and time and keeping them between 0 and 100
{
    energy=clampInt(energy+deltaEnergy,0,MAX_ENERGY);
    time=clampInt(time+deltaTime,0,MAX_TIME);
}

void applyScoreDelta(int& score, int deltaScore) //updating players score and keeping it between 0 and 520
{
    score=clampInt(score+deltaScore,0,MAX_SCORE);
}

bool hasItem(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount, char itemSymbol) //checking if a particular item exists in players inventory
{
    if(inventoryCount<0 || inventoryCount>MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    for(int i=0;i<inventoryCount;i++)
    {
        if(inventory[i]==itemSymbol)
        {
            return true;
        }
    }
    return false;
}

bool addItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol) //putting a new item at the end of the inventory
{
    if(itemSymbol==ENT_NONE) //checking if item is not empty
    {
        return false;
    }
    if(inventoryCount<0 || inventoryCount>=MAX_INVENTORY_CAPACITY) //checking if is full or count is invalid
    {
        return false;
    }
    if(hasItem(inventory,inventoryCount,itemSymbol)) //checking if the item already exists
    {
        return false;
    }
    inventory[inventoryCount]=itemSymbol; //adding new item after the checks
    inventoryCount++;
    return true; //item added successfully
}

bool removeItem(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, char itemSymbol) //taking one item out and shifting the rest to the left
{
    if(inventoryCount<=0 || inventoryCount>MAX_INVENTORY_CAPACITY)
    {
        return false;
    }
    int position=-1; //-1 because indexes start from 0 so -1 means item has not been found yet
    for(int i=0;i<inventoryCount;i++)
    {
        if(inventory[i]==itemSymbol)
        {
            position=i;
            break;
        }
    }
    if(position==-1) //item was not found
    {
        return false;
    }
    for(int i=position;i<inventoryCount-1;i++) //moving every item after it one step left to fill the gap
    {
        inventory[i]=inventory[i+1];
    }
    inventoryCount--;
    return true;
}
