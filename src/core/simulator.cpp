#include "simulator.h"
#include "physics.h"
#include "switches.h"
#include "logger.h"
#include "viewer_iface.h"
#include "util.h"

// Modular simulation loop using primitive variables
#include <iostream>
using namespace std;

//1. MOVE COMMAND
bool handleMoveCommand(int targetFloor, int targetRoom,int& currentFloor, int& currentRoom,int& energy, int& time,const char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,int& emergencyTripsLeft, bool lift2Operational)
{
// check for valid floor and room numbers
if (targetFloor < 1 || targetFloor > NUM_FLOORS ||targetRoom < 1 || targetRoom > ROOMS_PER_FLOOR)
 {
 cout<<"Invalid room or floor number is entered not exists"<<endl;
 return false;
 }

if (targetFloor == currentFloor && targetRoom == currentRoom)
{
cout <<"You are already in F"<< currentFloor << "-R" << currentRoom << ".\n";
return false;
 }

char connType = CONN_NONE;
 // Check if rooms are connected using physics.h here
if (!isDirectlyConnected(currentFloor, currentRoom, targetFloor, targetRoom, horizontalConn, connType))
{
 cout << "There is no direct path to F" << targetFloor << "-R" << targetRoom << ".\n";
 return false;
 }

//Check if player is allowed to pass using physics.h here
if (!canTraverse(targetFloor, targetRoom, connType, inventory, inventoryCount,emergencyTripsLeft, lift2Operational))
{
 cout << "You cannot move there may be blocked(locked door, wall, or lift unavailable)\n";
 return false;
 }

// Deduct emergency trip if using Lift 2 without normal power
if(currentRoom == 4 && targetRoom == 4 && currentFloor != targetFloor)
 {
    if (!lift2Operational && emergencyTripsLeft > 0) 
    {
        emergencyTripsLeft--;
        cout << "You have used one emergency trip. Remaining: " << emergencyTripsLeft << "\n";
    }
 }

// Updated position
int oldFloor = currentFloor;
currentFloor = targetFloor;
currentRoom = targetRoom;

// Applying energy & time cost using physics.h & config.h
if (connType == CONN_WINDOW) 
{
 applyResourceDelta(energy, time, COST_MOVE_WINDOW_ENERGY, COST_MOVE_WINDOW_TIME);
} 
else if (oldFloor != targetFloor && currentRoom != 1)
{
  // Lift 1 or Lift 2
  applyResourceDelta(energy, time, COST_MOVE_LIFT_ENERGY, COST_MOVE_LIFT_TIME);
 } 
 else 
 {
  // Ordinary door, unlocked door, or staircase
  applyResourceDelta(energy, time, COST_MOVE_ORDINARY_ENERGY, COST_MOVE_ORDINARY_TIME);
 }

  cout << "YOU have moved to F" << currentFloor << "-R" << currentRoom << ".\n";
  return true;
}

//2. INSPECT COMMAND
bool handleInspectCommand(int currentFloor, int currentRoom,int& energy, int& time, const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int entityCount, bool objectives[NUM_OBJECTIVES], int& score)
{
  applyResourceDelta(energy, time, COST_INSPECT_ENERGY, COST_INSPECT_TIME);
  // Showing room contents using viewer_iface.h
  renderRoomDetails(currentFloor, currentRoom, roomEntities, entityCount);

  int fIdx = currentFloor - 1;
  int rIdx = currentRoom - 1;
 // Check if any Clue Digit is in this room
 for (int i = 0; i < entityCount; i++)
 {
 char sym = roomEntities[fIdx][rIdx][i];
 if (sym == ENT_CLUE_4 && !objectives[OBJ_CLUE_4])
  {
  objectives[OBJ_CLUE_4] = true;
  applyScoreDelta(score, SCORE_CLUE_FOUND);
  cout << "Discovered clue digit 4! (+" << SCORE_CLUE_FOUND << " pts)\n";
   } 
  else if (sym == ENT_CLUE_2 && !objectives[OBJ_CLUE_2])
 {
  objectives[OBJ_CLUE_2] = true;
  applyScoreDelta(score, SCORE_CLUE_FOUND);
  cout << "Discovered clue digit 2! (+" << SCORE_CLUE_FOUND << " pts)\n";
  }
  else if (sym == ENT_CLUE_7 && !objectives[OBJ_CLUE_7])
  {
  objectives[OBJ_CLUE_7] = true;
  applyScoreDelta(score, SCORE_CLUE_FOUND);
  cout << "Discovered clue digit 7! (+" << SCORE_CLUE_FOUND << " pts)\n";
    }
}

return true;
}

//3. COLLECT COMMAND
bool handleCollectCommand(char itemSymbol, int currentFloor, int currentRoom,int& energy, int& time, int& score, char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int& entityCount,char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, bool objectives[NUM_OBJECTIVES])
{
int fIdx = currentFloor - 1;
int rIdx = currentRoom - 1;
// Find item in room
int foundIdx = -1;
for (int i = 0; i < entityCount; i++)
{
 if (roomEntities[fIdx][rIdx][i] == itemSymbol)
 {
  foundIdx = i;
  break;
 }
}

if (foundIdx == -1)
{
cout << "Item '" << itemSymbol << "' is not found in this room.\n";
return false;
}

// Non-collectible entities
if (itemSymbol == ENT_CLUE_4 || itemSymbol == ENT_CLUE_2 || itemSymbol == ENT_CLUE_7 ||itemSymbol == ENT_CONTROL_PANEL || itemSymbol == ENT_PASSENGER || itemSymbol == ENT_CAT)
{
 cout << "You can't pick up'" << itemSymbol << "'.\n";
 return false;
}
// Cabinet Train Key can only be collected after cabinet is unlocked
if (itemSymbol == ENT_CABINET_TRAIN_KEY && !objectives[OBJ_CABINET_UNLOCKED])
{
 cout << "The Security Cabinet is locked! Use 'unlock' with the 3-digit code first.\n";
 return false;
 }
// Remove item from room array
for (int i = foundIdx; i < entityCount - 1; i++)
{
roomEntities[fIdx][rIdx][i] = roomEntities[fIdx][rIdx][i + 1];
}
entityCount--;
// Apply pickup cost
applyResourceDelta(energy, time, COST_ITEM_PICKUP_ENERGY, COST_ITEM_PICKUP_TIME);
// Handle instant consumables
if (itemSymbol == ENT_ENERGY_PACK)
{
applyResourceDelta(energy, time, BONUS_ENERGY_PACK, 0);
cout << "Collected Energy Pack (+" << BONUS_ENERGY_PACK << " Energy)!\n";
return true;
}
if (itemSymbol == ENT_EMERGENCY_TOOLKIT)
{
 applyResourceDelta(energy, time, BONUS_EMERGENCY_TOOLKIT, 0);
 cout << "Collected Emergency Toolkit (+" << BONUS_EMERGENCY_TOOLKIT << " Energy)!\n";
 return true;
}
if (itemSymbol == ENT_OBSERVATION_TOKEN)
{
 applyResourceDelta(energy, time, 0, BONUS_OBSERVATION_TOKEN);
 cout << "Collected Observation Token (+" << BONUS_OBSERVATION_TOKEN << " Time)!\n";
 return true;
 }
// Add normal item to inventory using physics.h
if (!addItem(inventory, inventoryCount, itemSymbol)) 
{
cout << "Inventory is full man!\n";
        return false;
    }
// Update objectives and score
if (itemSymbol == ENT_MASTER_KEY)
{
objectives[OBJ_MASTER_KEY] = true;
applyScoreDelta(score, SCORE_KEY_COLLECT);
} 
else if (itemSymbol == ENT_FUSE)
{
 objectives[OBJ_FUSE_COLLECTED] = true;
}
else if (itemSymbol == ENT_LIFT_PASS)
{
 objectives[OBJ_LIFT_PASS] = true;
applyScoreDelta(score, SCORE_LIFT_PASS);
 if (objectives[OBJ_FUSE_INSTALLED]) 
 {
  objectives[OBJ_LIFT2_ACTIVE] = true;
     }
}

else if (itemSymbol == ENT_CAT_CARRIER)
{
 objectives[OBJ_CAT_CARRIER] = true;
    } 
else if (itemSymbol == ENT_CABINET_TRAIN_KEY)
{
 objectives[OBJ_TRAIN_KEY] = true;
 applyScoreDelta(score, SCORE_TRAIN_KEY);
 }
cout << "Collected '" << itemSymbol << "'.\n";
return true;
}

//4. USE COMMAND
bool handleUseCommand(char itemSymbol, int currentFloor, int currentRoom,int& energy, int& time, int& score,char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount, bool objectives[NUM_OBJECTIVES])
{
if (!hasItem(inventory, inventoryCount, itemSymbol)) 
{
cout << "You don't have '" << itemSymbol << "' in your inventory.\n";
return false;
}

// Install Fuse at Control Panel (F3-R4) using switches.h
if (itemSymbol == ENT_FUSE && currentFloor == 3 && currentRoom == 4)
{
 if (installFuseControlPanel(inventory, inventoryCount, objectives, score))
 {
  applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
    if (objectives[OBJ_LIFT_PASS])
    {
     objectives[OBJ_LIFT2_ACTIVE] = true;
    }
    cout << "Fuse installed! Lift 2 power restored.\n";
    return true;
    }
return false;
 }

// Treat Injured Passenger with the Medical Kit (F4-R1)
if (itemSymbol == ENT_MEDICAL_KIT && currentFloor == 4 && currentRoom == 1)
{
 if (objectives[OBJ_PASSENGER_RESCUED])
 {
 cout << "Passenger is already rescued.\n";
 return false;
 }
 objectives[OBJ_PASSENGER_RESCUED] = true;
 applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
 applyScoreDelta(score, SCORE_RESCUE_PASSENGER);
 cout << "Injured passenger rescued! (+" << SCORE_RESCUE_PASSENGER << " pts)\n";
 return true;
 }

// Secure Cat with Cat Carrier (F3-R3)
if (itemSymbol == ENT_CAT_CARRIER && currentFloor == 3 && currentRoom == 3)
{
 if (objectives[OBJ_CAT_SECURED])
 {
  cout << "Cat is already secured.\n";
  return false;
}

objectives[OBJ_CAT_SECURED] = true;
applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
applyScoreDelta(score, SCORE_SECURE_CAT);
cout << "Cat safely secured! (+" << SCORE_SECURE_CAT << " pts)\n";
return true;
}

// Prepare & Board Train with Train Key (F5-R2)
if (itemSymbol == ENT_CABINET_TRAIN_KEY && currentFloor == 5 && currentRoom == 2)
{
 if (!objectives[OBJ_PASSENGER_RESCUED] || !objectives[OBJ_CAT_SECURED])
 {
  cout << "You must rescue both  Passenger and the cat before boarding\n";
  return false;
 }
 objectives[OBJ_TRAIN_PREPARED] = true;
 objectives[OBJ_MISSION_WON] = true;
 applyResourceDelta(energy, time, COST_ITEM_USE_ENERGY, COST_ITEM_USE_TIME);
 applyScoreDelta(score, SCORE_PREPARE_TRAIN + SCORE_MISSION_VICTORY);
 cout<<"Train prepared and boarded! MISSION VICTORY!\n";
 return true;
}

cout << "Cannot use '" << itemSymbol << "' in F" << currentFloor << "-R" << currentRoom << ".\n";
return false;
}

//5. UNLOCK CABINET COMMAND
bool handleUnlockCabinetCommand(const char* code, int currentFloor, int currentRoom,bool objectives[NUM_OBJECTIVES], int& score,char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM], int& entityCount)
{
if (currentFloor != 5 || currentRoom != 1)
{
cout << "There is no security cabinet in this room.\n";
return false;
}
if (objectives[OBJ_CABINET_UNLOCKED])
{
cout << "Cabinet is already unlocked.\n";
return false;
}

// Verify code using switches.h
if (verifyCabinetCode(code, objectives))
{
objectives[OBJ_CABINET_UNLOCKED] = true;
applyScoreDelta(score, SCORE_CABINET_UNLOCK);
// Ensure train Key is present in F5-R1 to collect
int fIdx = currentFloor - 1; //fIdx is for floor index while rIdx is for room index
int rIdx = currentRoom - 1;
bool keyPresent = false;
for (int i = 0; i < entityCount; i++) 
{
 if (roomEntities[fIdx][rIdx][i] == ENT_CABINET_TRAIN_KEY)
 {
 keyPresent = true;
break;
 }
}

if (!keyPresent && entityCount < MAX_ENTITIES_PER_ROOM)
{
 roomEntities[fIdx][rIdx][entityCount] = ENT_CABINET_TRAIN_KEY;
 entityCount++;
}

cout << "Cabinet unlocked! (+" << SCORE_CABINET_UNLOCK << " pts) Collect '^' for Train Key.\n";
return true;
}
cout << "Wrong code or missing clues!\n";
return false;
}

//6. CHECK END CONDITIONS
bool checkEndConditions(int energy, int time, const bool objectives[NUM_OBJECTIVES])
{
if (objectives[OBJ_MISSION_WON])
{
 return true;
}
if (energy <= 0 || time <= 0 || objectives[OBJ_MISSION_FAILED])
{
 return true;
}
return false;
}

//7. MAIN SIMULATION LOOP
void runSimulation(int& playerFloor, int& playerRoom,int& energy, int& time, int& score,char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,char horizontalConn[NUM_FLOORS][ROOMS_PER_FLOOR - 1],char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
char cmd[MAX_NAME_LEN];
while (!checkEndConditions(energy, time, objectives))
{
// Display UI using viewer_iface.h
renderDashboard(playerFloor, playerRoom, energy, time, score,inventory, inventoryCount, emergencyTripsLeft);
cout << "Commands: move, inspect, collect, use, unlock, map, goals, save, load, quit\n";
cout << "Enter command: ";
cin >> cmd;
int fIdx = playerFloor - 1;
int rIdx = playerRoom - 1;

if (stringEquals(cmd, "move"))
{
int f, r;
cout << "Target Floor (1-5) and Room (1-4): ";
cin >> f >> r;
bool lift2Ready = objectives[OBJ_FUSE_INSTALLED] && objectives[OBJ_LIFT_PASS];
handleMoveCommand(f, r, playerFloor, playerRoom, energy, time, horizontalConn, inventory, inventoryCount,emergencyTripsLeft, lift2Ready);
} 
else if (stringEquals(cmd, "inspect"))
{
handleInspectCommand(playerFloor, playerRoom, energy, time,roomEntities, entityCounts[fIdx][rIdx],objectives, score);
} 
else if (stringEquals(cmd, "collect"))
{
char sym;
cout << "Enter item symbol: ";
cin >> sym;
if (handleCollectCommand(sym, playerFloor, playerRoom, energy, time, score,roomEntities, entityCounts[fIdx][rIdx], inventory, inventoryCount, objectives))
 {
 if (sym == ENT_EMERGENCY_CARD)
 {
  emergencyTripsLeft = 3;
  cout << "Emergency Access Card activated (3 Lift 2 trips).\n";
 }
}
}
else if (stringEquals(cmd, "use")) {
char sym;
cout << "Enter item symbol: ";
cin >> sym;
handleUseCommand(sym, playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, objectives);
} 
else if (stringEquals(cmd, "unlock"))
 {
  char code[MAX_NAME_LEN];
  cout << "Enter 3-digit cabinet code: ";
  cin >> code;
  handleUnlockCabinetCommand(code, playerFloor, playerRoom, objectives, score, roomEntities, entityCounts[fIdx][rIdx]);
  }
  else if (stringEquals(cmd, "map"))
  {
    renderAsciiBuilding(playerFloor, playerRoom);
   }
 else if (stringEquals(cmd, "goals"))
 {
 renderObjectivesChecklist(objectives);
 } 
 else if (stringEquals(cmd, "save"))
 {
  if (saveGameStateToFile("out/savegame.txt", playerFloor, playerRoom,energy, time, score, inventory, inventoryCount, objectives, emergencyTripsLeft, roomEntities, entityCounts)) {
  cout << "Game saved to out/savegame.txt.\n";
  }
} 
else if (stringEquals(cmd, "load")) 
{
 if (loadGameStateFromFile("out/savegame.txt", playerFloor, playerRoom, energy, time, score, inventory, inventoryCount, objectives, emergencyTripsLeft, roomEntities, entityCounts)) 
 {
 cout << "Game loaded from out/savegame.txt.\n";
 }
} 
else if (stringEquals(cmd, "quit"))
 {
 cout << "Exiting simulation...\n";
  break;
 } 
 else
  {
    cout << "Unknown command.\n";
    }
 }

// Final result & summary report using logger.h
 bool won = objectives[OBJ_MISSION_WON];
 if (!won && (energy <= 0 || time <= 0)) 
{
 objectives[OBJ_MISSION_FAILED] = true;
 cout << "\n*** MISSION FAILED! Out of Energy or Time. ***\n";
}
 else if (won) {
 cout << "\n*** CONGRATULATIONS! MISSION COMPLETE! Score: " << score << "/520 ***\n";
}

writeGameSummaryReport("out/game_summary.txt", score, energy, time, objectives, won);
}