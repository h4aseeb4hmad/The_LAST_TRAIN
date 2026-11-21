#include "switches.h"
#include "util.h"

// Door lock mechanics, lifts, and cabinet verification without structures
#include "physics.h"
#include <iostream>
using namespace std;

// Security Cabinet (F5-R1) ka code "427" aur saare clues verify karta hai
bool verifyCabinetCode(const char* enteredCode, const bool objectives[NUM_OBJECTIVES]) {
// Pehle check karein ke teeno clues mil chuke hain ya nahi
if (!objectives[OBJ_CLUE_4] || !objectives[OBJ_CLUE_2] || !objectives[OBJ_CLUE_7])
{
 cout << "You haven't discovered all the 3 clue digits yet\n";
 return false;
}
//just to check code util.h stringEquals function
if (stringEquals(enteredCode, "427")) 
{
 return true;
}
return false;
}

// Lift 1 is only operational between floor 1 and floor 2
bool canUseLift1(int Floor, int destinationFloor) 
{
 if (Floor == destinationFloor) 
 {
  return false;
 }

 else if ((Floor == 1 && destinationFloor == 2) || (Floor == 2 && destinationFloor == 1))
 {
  return true;
 }
else{
return false;
}
}
//Lift 2 is only operational on Floors 2, 4, and 5 (Floor 3 is bypassed)
bool canUseLift2(int Floor, int destinationfloor, bool lift2Operational, int emergencyTripsLeft)
 {
  if (Floor == destinationfloor)
 {
    return false;
    }

bool lift2FROM = (Floor == 2 || Floor == 4 || Floor == 5);
bool lift2going   = (destinationfloor == 2 || destinationfloor == 4 || destinationfloor == 5);

if (!lift2FROM || !lift2going)
 {
  return false;
 }

//lift 2 operational or emergency trips left
if (lift2Operational || emergencyTripsLeft > 0) {
return true;
}
return false;
}

//Use the master key to unlock the Floor 3 door
bool unlockFloor3Door(const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount) {
if (hasItem(inventory, inventoryCount, ENT_MASTER_KEY))
{
 return true;
}
return false;
}

//Control Panel (F3-R4) par Fuse lagane ka function
bool installFuseControlPanel(char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, bool objectives[NUM_OBJECTIVES], int& score)
{
if (objectives[OBJ_FUSE_INSTALLED])
 {
cout << "Fuse is already installed in the control panel.\n";
return false;
    }

if (!hasItem(inventory, inventoryCount, ENT_FUSE))
{
cout << "You don't have the fuse in your inventory.\n";
return false;
}

//Removing fuse from inventory and updating objectives and score.
removeItem(inventory, inventoryCount, ENT_FUSE);
objectives[OBJ_FUSE_INSTALLED] = true;
applyScoreDelta(score, SCORE_INSTALL_FUSE);
return true;
}
