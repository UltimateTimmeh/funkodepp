// A small game to practice pokemon type matchups.
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


struct Type
{
  std::string name;
  olc::Pixel active_color;
  olc::Pixel inactive_color;
  std::vector<float> defense_multipliers;
};


struct Cell
{
  float multiplier;
  bool active;
  Type type;
  int x;
  int y;
  bool selected = false;
};


int random_integer(int min, int max)
{
  // Return a random number between min and max (inclusive)
  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::uniform_int_distribution<> distr(min, max); // define the range
  return distr(gen);
}


// Override base class with your custom functionality
class olcPracticeTypeMatchups : public olc::PixelGameEngine
{
public:
  olcPracticeTypeMatchups()
  {
    // Name your application
    sAppName = "Practice Type Matchups";
  }

private:
  std::vector<Type> all_types;

  // The game mode.
  bool single_type_mode = true;

  // Effectiveness grid settings.
  int grid_width = 6;
  int grid_height = 3;

  // Effectiveness cell settings.
  int cell_width = 70;
  int cell_height = 22;
  int grid_spacing = 12;
  int grid_x_offset = 640 - grid_spacing - grid_width * cell_width;

  // The six effectiveness grids.
  std::vector<Cell> grid_4x;
  std::vector<Cell> grid_2x;
  std::vector<Cell> grid_1x;
  std::vector<Cell> grid_05x;
  std::vector<Cell> grid_025x;
  std::vector<Cell> grid_0x;

  Cell* get_cell(int index_type, int index_grid)
  {
    Cell* cell;
    switch (index_grid)
    {
      case 0:
        cell = &grid_4x[index_type];
        break;
      case 1:
        cell = &grid_2x[index_type];
        break;
      case 2:
        cell = &grid_1x[index_type];
        break;
      case 3:
        cell = &grid_05x[index_type];
        break;
      case 4:
        cell = &grid_025x[index_type];
        break;
      case 5:
        cell = &grid_0x[index_type];
        break;
    }
    return cell;
  }

  // The defender's randomly chosen types.
  Type* defender_1_type;
  Type* defender_2_type;

  // Defending cell settings.
  int defender_width = 150;
  int defender_height = 40;
  int defender_y_offset = grid_spacing;
  int defender_x_pos = grid_spacing;
  int defender_1_y_pos = defender_y_offset;
  int defender_2_y_pos = defender_1_y_pos + defender_height + grid_spacing;

  // Mode button settings.
  int mode_width = defender_width;
  int mode_height = defender_height;
  int mode_x_pos = defender_x_pos;
  int mode_y_pos = defender_2_y_pos + defender_height + grid_spacing;
  std::string mode_text = (single_type_mode) ? "SINGLE" : "DOUBLE";
  bool mode_selected = false;

  // New game button settings.
  int new_game_width = defender_width;
  int new_game_height = defender_height;
  int new_game_x_pos = defender_x_pos;
  int new_game_y_pos = 480 - grid_spacing - new_game_height;
  std::string new_game_text = "NEW GAME";
  bool new_game_selected = false;
  olc::Pixel new_game_color = olc::VERY_DARK_GREY;

  // Solve button settings.
  int solve_width = defender_width;
  int solve_height = defender_height;
  int solve_x_pos = defender_x_pos;
  int solve_y_pos = new_game_y_pos - grid_spacing - solve_height;
  std::string solve_text = "SOLVE";
  bool solve_selected = false;
  olc::Pixel solve_color = olc::DARK_GREY;

  // Check button settings.
  int check_width = defender_width;
  int check_height = defender_height;
  int check_x_pos = defender_x_pos;
  int check_y_pos = solve_y_pos - grid_spacing - check_height;
  std::string check_text = "CHECK";
  bool check_selected = false;
  olc::Pixel check_color_neutral = olc::GREY;
  olc::Pixel check_color_correct = olc::DARK_GREEN;
  olc::Pixel check_color_incorrect = olc::DARK_RED;
  olc::Pixel* check_color = &check_color_neutral;

  // Functions for getting expected and selected multiplyer arrays and checking if they are the same.
  void get_expected_multipliers(std::vector<float>& expected)
  {
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      float multiplier = defender_1_type->defense_multipliers[i] * defender_2_type->defense_multipliers[i];
      expected.push_back(multiplier);
    }
  }

  void get_selected_multipliers(std::vector<float>& selected)
  {
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      for (int g = 0; g < 6; ++g)
      {
        Cell* cell = get_cell(i, g);
        if (cell->active)
        {
          selected.push_back(cell->multiplier);
          break;
        }
      }
    }
  }

  bool expected_equal_to_selected()
  {
    std::vector<float> expected;
    std::vector<float> selected;
    get_expected_multipliers(expected);
    get_selected_multipliers(selected);
    bool is_equal = true;
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      if (expected[i] != selected[i])
      {
        is_equal = false;
        break;
      }
    }
    return is_equal;
  }

  void show_solution()
  {
    std::vector<float> expected;
    get_expected_multipliers(expected);
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      int g;
      float multiplier = expected[i];
      if (multiplier == 4.0f)
        g = 0;
      else if (multiplier == 2.0f)
        g = 1;
      else if (multiplier == 1.0f)
        g = 2;
      else if (multiplier == 0.5f)
        g = 3;
      else if (multiplier == 0.25f)
        g = 4;
      else
        g = 5;
      get_cell(i, g)->active = true;
      for (int j = 0; j < 6; ++j)
        if (g != j) get_cell(i, j)->active = false;
    }
    check_color = (expected_equal_to_selected()) ? &check_color_correct : &check_color_incorrect;
  }

  void choose_new_defender_types()
  {
    int itype_1 = random_integer(0, 17);
    int itype_2 = all_types.size()-1;
    if (!single_type_mode)
    {
      itype_2 = itype_1;
      while (itype_1 == itype_2)
        itype_2 = random_integer(0, 18);
    }
    defender_1_type = &all_types[itype_1];
    defender_2_type = &all_types[itype_2];
  }

  void reset_effectiveness_cells()
  {
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      for (int g = 0; g < 6; ++g)
      {
        if (g == 2)
          get_cell(i, g)->active = true;
        else
          get_cell(i, g)->active = false;
      }
    }
  }

  void new_game()
  {
    choose_new_defender_types();
    reset_effectiveness_cells();
    check_color = &check_color_neutral;
  }

  void toggle_game_mode()
  {
    single_type_mode = !single_type_mode;
    mode_text = (single_type_mode) ? "SINGLE" : "DOUBLE";
    new_game();
  }

public:
  bool OnUserCreate() override
  {
    // Initialize all Pokemon types.
    all_types = {
      { "Normal", { 147, 158, 160 }, { 60, 60, 60 }, { 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f } },
      { "Fighting", { 197, 68, 109 }, { 60, 60, 60 }, { 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 0.5f, 2.0f } },
      { "Flying", { 143, 173, 227 }, { 60, 60, 60 }, { 1.0f, 0.5f, 1.0f, 1.0f, 0.0f, 2.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f } },
      { "Poison", { 168, 106, 207 }, { 60, 60, 60 }, { 1.0f, 0.5f, 1.0f, 0.5f, 2.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 0.5f } },
      { "Ground", { 207, 123, 59 }, { 60, 60, 60 }, { 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 0.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f } },
      { "Rock", { 197, 184, 132 }, { 60, 60, 60 }, { 0.5f, 2.0f, 0.5f, 0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0.5f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f } },
      { "Bug", { 153, 187, 28 }, { 60, 60, 60 }, { 1.0f, 0.5f, 2.0f, 1.0f, 0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f } },
      { "Ghost", { 83, 109, 166 }, { 60, 60, 60 }, { 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f } },
      { "Steel", { 88, 145, 164 }, { 60, 60, 60 }, { 0.5f, 2.0f, 0.5f, 0.0f, 2.0f, 0.5f, 0.5f, 1.0f, 0.5f, 2.0f, 1.0f, 0.5f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f } },
      { "Fire", { 255, 155, 77 }, { 60, 60, 60 }, { 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 0.5f, 1.0f, 0.5f, 0.5f, 2.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 0.5f } },
      { "Water", { 93, 148, 212 }, { 60, 60, 60 }, { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 2.0f, 2.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f } },
      { "Grass", { 117, 188, 68 }, { 60, 60, 60 }, { 1.0f, 1.0f, 2.0f, 2.0f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f, 2.0f, 0.5f, 0.5f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f } },
      { "Electric", { 243, 208, 54 }, { 60, 60, 60 }, { 1.0f, 1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f } },
      { "Psychic", { 241, 110, 118 }, { 60, 60, 60 }, { 1.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 2.0f, 1.0f } },
      { "Ice", { 131, 203, 192 }, { 60, 60, 60 }, { 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f } },
      { "Dragon", { 40, 110, 198 }, { 60, 60, 60 }, { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 0.5f, 1.0f, 2.0f, 2.0f, 1.0f, 2.0f } },
      { "Dark", { 89, 87, 100 }, { 60, 60, 60 }, { 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 2.0f } },
      { "Fairy", { 218, 151, 231 }, { 60, 60, 60 }, { 1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f, 0.5f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f } },
      { "None", { 0, 0, 0 }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f } },
    };

    // Choose random defender types.
    choose_new_defender_types();

    // Populate the effectiveness grids.
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      int ix = i % grid_width;
      int iy = i / grid_width;
      int cell_x_position = ix * cell_width + grid_x_offset;
      int cell_y_position = iy * cell_height;
      for (int g = 0; g < 6; ++g)
      {
        int grid_y_offset = (g+1) * grid_spacing + g * grid_height * cell_height;
        switch (g)
        {
          case 0:
            grid_4x.push_back({ 4.0f, false, all_types[i], cell_x_position, cell_y_position + grid_y_offset });
            break;
          case 1:
            grid_2x.push_back({ 2.0f, false, all_types[i], cell_x_position, cell_y_position + grid_y_offset });
            break;
          case 2:
            grid_1x.push_back({ 1.0f, true, all_types[i], cell_x_position, cell_y_position + grid_y_offset });
            break;
          case 3:
            grid_05x.push_back({ 0.5f, false, all_types[i], cell_x_position, cell_y_position + grid_y_offset });
            break;
          case 4:
            grid_025x.push_back({ .25f, false, all_types[i], cell_x_position, cell_y_position + grid_y_offset });
            break;
          case 5:
            grid_0x.push_back({ 0.0f, false, all_types[i], cell_x_position, cell_y_position + grid_y_offset });
            break;
        }
      }
    }

    return true;
  }

  bool OnUserUpdate(float fElapsedTime) override
  {
    // Determine if an effectiveness cell is currently selected.
    int mouse_x = GetMouseX();
    int mouse_y = GetMouseY();
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      for (int g = 0; g < 6; ++g)
      {
        Cell* cell = get_cell(i, g);
        cell->selected = (mouse_x > cell->x && mouse_x < cell->x + cell_width &&
                          mouse_y > cell->y && mouse_y < cell->y + cell_height);
      }
    }

    // Determine if the game mode button is currently selected.
    mode_selected = (mouse_x > mode_x_pos && mouse_x < mode_x_pos + mode_width &&
                     mouse_y > mode_y_pos && mouse_y < mode_y_pos + mode_height);

    // Determine if the check button is currently selected.
    check_selected = (mouse_x > check_x_pos && mouse_x < check_x_pos + check_width &&
                      mouse_y > check_y_pos && mouse_y < check_y_pos + check_height);

    // Determine if the solve button is currently selected.
    solve_selected = (mouse_x > solve_x_pos && mouse_x < solve_x_pos + solve_width &&
                     mouse_y > solve_y_pos && mouse_y < solve_y_pos + solve_height);

    // Determine if the new game button is currently selected.
    new_game_selected = (mouse_x > new_game_x_pos && mouse_x < new_game_x_pos + new_game_width &&
                        mouse_y > new_game_y_pos && mouse_y < new_game_y_pos + new_game_height);

    // On left mouse button release.
    if (GetMouse(0).bReleased)
    {
      // Toggle activeness of currently selected effectiveness cell (if any).
      for (int i = 0; i < all_types.size()-1; ++i)
      {
        for (int g = 0; g < 6; ++g)
        {
          Cell* cell = get_cell(i, g);
          if (cell->selected)
          {
            if (cell->active)
            {
              // If the cell is active, make it inactive and make the corresponding cell in the
              // times one effectiveness grid active.
              cell->active = false;
              get_cell(i, 2)->active = true;
            } else
            {
              // Else make the cell active and make sure the corresponding cells in all other
              // effectiveness grids are inactive.
              cell->active = true;
              if (g != 0) get_cell(i, 0)->active = false;
              if (g != 1) get_cell(i, 1)->active = false;
              if (g != 2) get_cell(i, 2)->active = false;
              if (g != 3) get_cell(i, 3)->active = false;
              if (g != 4) get_cell(i, 4)->active = false;
              if (g != 5) get_cell(i, 5)->active = false;
            }
            break;
          }
        }
      }

      // Toggle game mode (if game mode button selected).
      if (mode_selected)
      {
        toggle_game_mode();
      }

      // Trigger check (if check button selected).
      if (check_selected)
      {
        check_color = (expected_equal_to_selected()) ? &check_color_correct : &check_color_incorrect;
      }

      // Trigger solve (if solve button selected).
      if (solve_selected)
      {
        show_solution();
      }

      // Trigger new game (if new game button selected).
      if (new_game_selected)
      {
        new_game();
      }
    }

    // Draw all the things.
    Clear(olc::BLACK);

    // Draw the effectiveness grids.
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      for (int g = 0; g < 6; ++g)
      {
        Cell* cell = get_cell(i, g);
        olc::Pixel background_color = cell->active ? cell->type.active_color : cell->type.inactive_color;
        olc::Pixel text_color = cell->active ? olc::WHITE : olc::BLACK;
        FillRect(cell->x, cell->y, cell_width, cell_height, background_color);
        DrawRect(cell->x, cell->y, cell_width, cell_height, olc::BLACK);
        DrawString(cell->x+4, cell->y+7, cell->type.name, text_color);
      }
    }

    // Draw the multiplier in front of each effectiveness grid.
    for (int g = 0; g < 6; ++g)
    {
      Cell* cell = get_cell(0, g);
      std::ostringstream oss;
      oss << std::setprecision(2) << std::noshowpoint << cell->multiplier;
      std::string multiplier_text = oss.str() + "x";
      DrawString(cell->x-8*multiplier_text.size()-2, cell->y+7, multiplier_text);
    }

    // Draw the selected effectiveness cell.
    for (int i = 0; i < all_types.size()-1; ++i)
    {
      for (int g = 0; g < 6; ++g)
      {
        Cell* cell = get_cell(i, g);
        if (cell->selected)
          DrawRect(cell->x, cell->y, cell_width, cell_height, olc::RED);
      }
    }

    // Draw the defender types.
    FillRect(defender_x_pos, defender_1_y_pos, defender_width, defender_height, defender_1_type->active_color);
    DrawRect(defender_x_pos, defender_1_y_pos, defender_width, defender_height, olc::WHITE);
    DrawString(defender_x_pos+8, defender_1_y_pos+14, defender_1_type->name, olc::WHITE, 2);

    FillRect(defender_x_pos, defender_2_y_pos, defender_width, defender_height, defender_2_type->active_color);
    DrawRect(defender_x_pos, defender_2_y_pos, defender_width, defender_height, olc::WHITE);
    DrawString(defender_x_pos+8, defender_2_y_pos+14, defender_2_type->name, olc::WHITE, 2);

    // Draw the game mode button.
    if (mode_selected)
      DrawRect(mode_x_pos, mode_y_pos, mode_width, mode_height, olc::RED);
    else
      DrawRect(mode_x_pos, mode_y_pos, mode_width, mode_height, olc::WHITE);
    DrawString(mode_x_pos+8, mode_y_pos+14, mode_text, olc::WHITE, 2);

    // Draw the check button.
    FillRect(check_x_pos, check_y_pos, check_width, check_height, *check_color);
    if (check_selected)
      DrawRect(check_x_pos, check_y_pos, check_width, check_height, olc::RED);
    else
      DrawRect(check_x_pos, check_y_pos, check_width, check_height, olc::WHITE);
    DrawString(check_x_pos+8, check_y_pos+14, check_text, olc::WHITE, 2);

    // Draw the solve button.
    FillRect(solve_x_pos, solve_y_pos, solve_width, solve_height, solve_color);
    if (solve_selected)
      DrawRect(solve_x_pos, solve_y_pos, solve_width, solve_height, olc::RED);
    else
      DrawRect(solve_x_pos, solve_y_pos, solve_width, solve_height, olc::WHITE);
    DrawString(solve_x_pos+8, solve_y_pos+14, solve_text, olc::WHITE, 2);

    // Draw the new game button.
    FillRect(new_game_x_pos, new_game_y_pos, new_game_width, new_game_height, new_game_color);
    if (new_game_selected)
      DrawRect(new_game_x_pos, new_game_y_pos, new_game_width, new_game_height, olc::RED);
    else
      DrawRect(new_game_x_pos, new_game_y_pos, new_game_width, new_game_height, olc::WHITE);
    DrawString(new_game_x_pos+8, new_game_y_pos+14, new_game_text, olc::WHITE, 2);

    return true;
  }
};

int main()
{
  olcPracticeTypeMatchups game;
  if (game.Construct(640, 480, 2, 2))
    game.Start();
  return 0;
}
