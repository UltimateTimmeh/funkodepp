// A small game to practice pokemon type matchups.
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


// Override base class with your custom functionality
class olcTypeMatchupGrid : public olc::PixelGameEngine
{
public:
  olcTypeMatchupGrid()
  {
    // Name your application
    sAppName = "Type Matchup Grid";
  }

private:

  // Grid settings.
  int grid_x = 140;
  int grid_y = 60;
  int cell_size = 20;

  // Cell states.
  std::string cell_states[4] = { "0.0x", "0.5x", "1.0x", "2.0x" };

  // Default order of types.
  std::string default_order[18] = {
    "Normal", "Fighting", "Flying", "Poison", "Ground", "Rock",
    "Bug", "Ghost", "Steel", "Fire", "Water", "Grass",
    "Electric", "Psychic", "Ice", "Dragon", "Dark", "Fairy"
  };

  // Default type matchup grid.
  int default_grid[18][18] = {
  // No Fg Fl Po Gn Ro Bu Gh St Fr Wa Gs El Ps Ic Dg Dk Fa
    {2, 2, 2, 2, 2, 1, 2, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2}, // Normal
    {3, 2, 1, 1, 2, 3, 1, 0, 3, 2, 2, 2, 2, 1, 3, 2, 3, 1}, // Fighting
    {2, 3, 2, 2, 2, 1, 3, 2, 1, 2, 2, 3, 1, 2, 2, 2, 2, 2}, // Flying
    {2, 2, 2, 1, 1, 1, 2, 1, 0, 2, 2, 3, 2, 2, 2, 2, 2, 3}, // Poison
    {2, 2, 0, 3, 2, 3, 1, 2, 3, 3, 2, 1, 3, 2, 2, 2, 2, 2}, // Ground
    {2, 1, 3, 2, 1, 2, 3, 2, 1, 3, 2, 2, 2, 2, 3, 2, 2, 2}, // Rock
    {2, 1, 1, 1, 2, 2, 2, 1, 1, 1, 2, 3, 2, 3, 2, 2, 3, 1}, // Bug
    {0, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 3, 2, 2, 1, 2}, // Ghost
    {2, 2, 2, 2, 2, 3, 2, 2, 1, 1, 1, 2, 1, 2, 3, 2, 2, 3}, // Steel
    {2, 2, 2, 2, 2, 1, 3, 2, 3, 1, 1, 3, 2, 2, 3, 1, 2, 2}, // Fire
    {2, 2, 2, 2, 3, 3, 2, 2, 2, 3, 1, 1, 2, 2, 2, 1, 2, 2}, // Water
    {2, 2, 1, 1, 3, 3, 1, 2, 1, 1, 3, 1, 2, 2, 2, 1, 2, 2}, // Grass
    {2, 2, 3, 2, 0, 2, 2, 2, 2, 2, 3, 1, 1, 2, 2, 1, 2, 2}, // Electric
    {2, 3, 2, 3, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 0, 2}, // Psychic
    {2, 2, 3, 2, 3, 2, 2, 2, 1, 1, 1, 3, 2, 2, 1, 3, 2, 2}, // Ice
    {2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 3, 2, 0}, // Dragon
    {2, 1, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 3, 2, 2, 1, 1}, // Dark
    {2, 3, 2, 1, 2, 2, 2, 2, 1, 1, 2, 2, 2, 2, 2, 3, 3, 2}, // Fairy
  };

  // Game type matchup grid.
  int game_grid[18][18];

public:
  bool OnUserCreate() override
  {
    // Populate the game grid.
    for (int r = 0; r < 18; ++r)
      for (int c = 0; c < 18; ++c)
        game_grid[r][c] = 2;
    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // Get the row and column indices of the currently selected cell.
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    int selected_r = (mouse_y - grid_y) / cell_size;
    int selected_c = (mouse_x - grid_x) / cell_size;
    bool cell_selected = (mouse_x > grid_x && mouse_x < 18*cell_size+grid_x &&
                          mouse_y > grid_y && mouse_y < 18*cell_size+grid_y);

    // On left mouse button release.
    if (GetMouse(0).bReleased)
    {
      // Increase the state of the selected cell (if any).
      if (cell_selected)
      {
        int cell_state = game_grid[selected_r][selected_c];
        game_grid[selected_r][selected_c] = std::min(cell_state+1, 3);
      }
    }

    // On right mouse button release.
    if (GetMouse(1).bReleased)
    {
      // Decrease the state of the selected cell (if any).
      if (cell_selected)
      {
        int cell_state = game_grid[selected_r][selected_c];
        game_grid[selected_r][selected_c] = std::max(cell_state-1, 0);
      }
    }

    // Draw all the things.
    Clear(olc::BLACK);

    // Draw the game grid.
    for (int r = 0; r < 18; ++r)
    {
      for (int c = 0; c < 18; ++c)
      {
        int cell_state = game_grid[r][c];
        olc::Pixel cell_color;
        switch (cell_state)
        {
          case 0:
            cell_color = olc::BLUE;
            break;
          case 1:
            cell_color = olc::RED;
            break;
          case 2:
            cell_color = olc::WHITE;
            break;
          case 3:
            cell_color = olc::GREEN;
            break;
        }
        int cell_x = c*cell_size + grid_x;
        int cell_y = r*cell_size + grid_y;
        FillRect(cell_x, cell_y, cell_size, cell_size, cell_color);
        DrawRect(cell_x, cell_y, cell_size, cell_size, olc::BLACK);
      }
    }

    // Highlight the currently selected cell.
    if (cell_selected){
      int cell_x = selected_c*cell_size + grid_x;
      int cell_y = selected_r*cell_size + grid_y;
      SetPixelMode(olc::Pixel::ALPHA);
      FillRect(cell_x, cell_y, cell_size, cell_size, { 0, 0, 0, 64 });
      SetPixelMode(olc::Pixel::NORMAL);
      DrawRect(cell_x, cell_y, cell_size, cell_size, olc::BLACK);
    }


    return true;
  }
};

int main()
{
  olcTypeMatchupGrid game;
  if (game.Construct(640, 480, 1, 1))
    game.Start();
  return 0;
}
