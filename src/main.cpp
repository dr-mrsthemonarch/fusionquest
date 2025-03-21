// main.cpp
#include "ui.h"
#include "character.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ctime>
#include <cstdlib>

int main() {
  // Initialize random seed for stat rolling
  std::srand(std::time(nullptr));

  // Create character and page state
  Character character;
  int current_page = MAIN_MENU;

  // Create UI
  auto ui = CreateUI(&current_page, &character);

  // Run screen
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  screen.Loop(ui);

  return 0;
}