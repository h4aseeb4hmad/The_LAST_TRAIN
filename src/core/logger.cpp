#include "logger.h"
#include "util.h"

// File persistence for savegame.txt and game_summary.txt using primitive variables
#include <iostream>
#include <fstream>

using namespace std;

//saving and loading game state to/from a file.
bool saveGameStateToFile(const char* filename,int playerFloor, int playerRoom,int energy, int time, int score,const char inventory[MAX_INVENTORY_CAPACITY], int inventoryCount,const bool objectives[NUM_OBJECTIVES], int emergencyTripsLeft,const char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],const int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
 ofstream out(filename);
 if (!out)
 {
  cout << "Error: Could not open " << filename << " for saving.\n";
  return false; 
 }

 // Basic player stats
 out << playerFloor << " " << playerRoom << "\n";
 out << energy << " " << time << " " << score << " " << emergencyTripsLeft << "\n";

 // Inventory
 out << inventoryCount << "\n";
 for (int i = 0; i < inventoryCount; i++)
 {
  out << inventory[i] << " ";
    }
  out << "\n";

 // Objectives (0 = false, 1 = true)
 for (int i = 0; i < NUM_OBJECTIVES; i++)
 {
  out << (objectives[i] ? 1 : 0) << " ";
 }
  out << "\n";

 // Room entities
 for (int f = 0; f < NUM_FLOORS; f++)
 {
 for (int r = 0; r < ROOMS_PER_FLOOR; r++)
    {
     out << entityCounts[f][r];
    for (int k = 0; k < entityCounts[f][r]; k++)
        {
          out << " " << roomEntities[f][r][k];
            }
         out << "\n";
        }
    }
    out.close();
    return true;
}

//LOAD GAME STATE
bool loadGameStateFromFile(const char* filename,int& playerFloor, int& playerRoom,int& energy, int& time, int& score,
char inventory[MAX_INVENTORY_CAPACITY], int& inventoryCount,bool objectives[NUM_OBJECTIVES], int& emergencyTripsLeft,
char roomEntities[NUM_FLOORS][ROOMS_PER_FLOOR][MAX_ENTITIES_PER_ROOM],int entityCounts[NUM_FLOORS][ROOMS_PER_FLOOR])
{
 ifstream in(filename);
 if (!in)
 {
 cout << "Error: Could not open " << filename << " for loading.\n";
 return false;
    }

 // Basic player stats
 in >> playerFloor >>playerRoom;
 in >> energy>> time >> score >>emergencyTripsLeft;

 // Safe bounds using util.h (clampInt)
 playerFloor = clampInt(playerFloor, 1, NUM_FLOORS);
 playerRoom = clampInt(playerRoom, 1, ROOMS_PER_FLOOR);
 energy = clampInt(energy, 0, MAX_ENERGY);
 time = clampInt(time, 0, 100);

 // Inventory
 in >> inventoryCount;
 inventoryCount = clampInt(inventoryCount, 0, MAX_INVENTORY_CAPACITY);

 for (int i = 0; i < inventoryCount; i++)
 {
    in >> inventory[i];
    }

 // Objectives
 for (int i=0; i < NUM_OBJECTIVES; i++)
 {
  int flag =0;
  in >> flag;
  objectives[i] = (flag != 0);
    }

 // Room entities
 for (int f=0; f < NUM_FLOORS; f++)
 {
  for (int r=0; r < ROOMS_PER_FLOOR; r++)
    {
     in >> entityCounts[f][r];
     entityCounts[f][r] = clampInt(entityCounts[f][r], 0, MAX_ENTITIES_PER_ROOM);
     for (int k =0; k < entityCounts[f][r]; k++)
     {
     in >> roomEntities[f][r][k];
     }
    }
}

in.close();
return true;
}

//WRITE GAME SUMMARY
void writeGameSummaryReport(const char* filename,int score, int remainingEnergy, int remainingTime,const bool objectives[NUM_OBJECTIVES], bool victory)
{
 ofstream out(filename);
 if (!out)
 {
 cout << "Error: Could not open " << filename << " to write summary.\n";
 return;
 }

 out << "=========================================\n";
 out << "       THE LAST TRAIN - GAME SUMMARY     \n";
 out << "=========================================\n";
 out << "Mission Result : " << (victory ? "VICTORY" : "DEFEAT") << "\n";
 out << "Final Score : " << score << " / 520\n";
 out << "Remaining Energy : " << remainingEnergy << "\n";
 out << "Remaining Time : " << remainingTime << "\n";
 out << "\n--- Objectives Checklist ---\n";
 out << "Master Key Collected : " << (objectives[OBJ_MASTER_KEY] ? "Yes" : "No") << "\n";
 out << "Clue Digit 4 Found : " << (objectives[OBJ_CLUE_4] ? "Yes" : "No") << "\n";
 out << "Clue Digit 2 Found : " << (objectives[OBJ_CLUE_2] ? "Yes" : "No") << "\n";
 out << "Clue Digit 7 Found : " << (objectives[OBJ_CLUE_7] ? "Yes" : "No") << "\n";
 out << "Cabinet Unlocked : " << (objectives[OBJ_CABINET_UNLOCKED] ? "Yes" : "No") << "\n";
 out << "Train Key Acquired : " << (objectives[OBJ_TRAIN_KEY] ? "Yes" : "No") << "\n";
 out << "Fuse Installed : " << (objectives[OBJ_FUSE_INSTALLED] ? "Yes" : "No") << "\n";
 out << "Lift Pass Collected : " << (objectives[OBJ_LIFT_PASS] ? "Yes" : "No") << "\n";
 out << "Passenger Rescued : " << (objectives[OBJ_PASSENGER_RESCUED] ? "Yes" : "No") << "\n";
 out << "Cat Secured : " << (objectives[OBJ_CAT_SECURED] ? "Yes" : "No") << "\n";
 out << "Train Prepared : " << (objectives[OBJ_TRAIN_PREPARED] ? "Yes" : "No") << "\n";
 out << "=========================================\n";

 out.close();
}


