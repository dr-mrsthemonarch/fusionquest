#ifndef NAVBAR_H
#define NAVBAR_H

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

class GameEngine;

// shared bottom button bar: Back to Main | Save | (other two of Stats/Battle Log/Special Items) | Quit
ftxui::Component CreateNavBar(int* selected_page, GameEngine* engine, ftxui::Closure exit_closure, int current_page);

#endif
