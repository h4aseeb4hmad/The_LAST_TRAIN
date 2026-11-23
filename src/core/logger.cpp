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