#include "character.h"
#include "globals.h"
#include "progress_bar.h"
#include "ui.h"
#include <chrono>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

class CountdownComponent : public ComponentBase {
public:
  CountdownComponent() {
    start_time_ = std::chrono::system_clock::now();
    target_time_ = start_time_ + std::chrono::hours(24 * 365 * 20); // 20 years
  }

  [[nodiscard]] std::string GetCountdownString() const {
    auto now = std::chrono::system_clock::now();
    auto remaining =
        std::chrono::duration_cast<std::chrono::seconds>(target_time_ - now);

    if (remaining.count() <= 0) {
      return "Fusion Quest - LAUNCH TIME!";
    }

    auto seconds = remaining.count();
    auto minutes = seconds / 60;
    seconds %= 60;
    auto hours = minutes / 60;
    minutes %= 60;
    auto days = hours / 24;
    hours %= 24;
    auto months = days / 30;
    days %= 30; // Approximation
    auto years = months / 12;
    months %= 12;

    return "Fusion Quest - Power Plant Comissioning: " + std::to_string(years) +
           "y " + std::to_string(months) + "m " + std::to_string(days) + "d " +
           std::to_string(hours) + "h " + std::to_string(minutes) + "m " +
           std::to_string(seconds) + "s";
  }

  Element Render() {
    // Force an animation frame update every second
    if (animation_frame_++ % 60 == 0) {
      // Assuming ~60fps, update every second
      // The screen will refresh automatically
    }
    return text(GetCountdownString()) | bold | center;
  }

private:
  std::chrono::system_clock::time_point start_time_;
  std::chrono::system_clock::time_point target_time_;
  int animation_frame_ = 0;
};

int selected_equipment_index = 0;
int selected_patent_index = 0;
int selected_inventory_index = 0;

Component CreateStatsPage(int *selected_page, Character *character,
                          Closure exit_closure) {
  // Create progress bars for experience and progress
  auto experience_bar = std::make_shared<ProgressBar>(1.0f, 0.01f, true);
  auto progress_bar = std::make_shared<ProgressBar>(0.07f, 0.005f, true);
  auto encumberance_bar = std::make_shared<ProgressBar>(0.1f, 0.001f, true);

  // Create countdown timer
  auto countdown_timer = Make<CountdownComponent>();

  // Start the progress bars
  experience_bar->Start();
  encumberance_bar->Start();
  progress_bar->Start();

  auto back_button =
      Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }) |
      size(HEIGHT, EQUAL, 3);
  auto save_button =
      Button("Save", [selected_page] { *selected_page = MAIN_MENU; }) |
      size(HEIGHT, EQUAL, 3);
  auto quit_button =
      Button("Quit", std::move(exit_closure)) | size(HEIGHT, EQUAL, 3);

  // Create a container for equipment, patents
  auto equipment_items = Menu(&equipment_list, &selected_equipment_index);
  auto patent_items = Menu(&patent_list, &selected_patent_index);
  auto inventory_items = Menu(&inventory_list, &selected_inventory_index);

  // Create a container that includes all components
  auto container =
      Container::Vertical({back_button, save_button, quit_button,
                           equipment_items, patent_items, inventory_items});

  auto renderer = Renderer(container, [back_button, save_button, quit_button,
                                       character, experience_bar, progress_bar,
                                       encumberance_bar, equipment_items,
                                       patent_items, inventory_items,
                                       countdown_timer] {
    // Column 1: Character Stats

    Element character_stats =
        vbox({
            text("Character") | bold | center,
            separator(),
            hbox({text("Name: ") | bold,
                  text(character->name.empty() ? "Unknown" : character->name)}),
            hbox({text("Employee nr: ") | bold, text("57")}),
            hbox({text("Position: ") | bold,
                  text(RACES[character->race_index])}),
            hbox({text("Startup Name: ") | bold,
                  text(FUSION_STARTUPS[character->fusion_index])}),
            // separator(),
            // hbox({text("Weapon: ") | bold, text("Vintage Macbook")}),
            // hbox({text("Dress: ") | bold, text("Smelly Sweater")}),
            // hbox({text("Accessory: ") | bold,
            //       text("Battery powered heated mug")}),
            separator(),
            hbox({text("Runway: ") | bold,
                  text(std::to_string(int(character->health * 100)) + "/100")}),
            hbox({text("Hype Energy: ") | bold,
                  text(std::to_string(int(character->mana * 100)) + "/100")}),
            hbox({text("Income: ") | bold,
                  text(std::to_string(int(character->mana * 100)) + "$")}),
            separator(),
            hbox({text("Hustle: ") | bold,
                  text(std::to_string(character->hustle))}),
            hbox({text("Agile-ity: ") | bold,
                  text(std::to_string(character->agility))}),
            hbox(
                {text("NDAs: ") | bold, text(std::to_string(character->ndas))}),
            hbox(
                {text("Luck: ") | bold, text(std::to_string(character->luck))}),
            hbox({text("Synergy: ") | bold,
                  text(std::to_string(character->Synergy))}),
            hbox({text("Technical Debt: ") | bold,
                  text(std::to_string(character->techdebt))}),
            hbox({text("Bus Factor: ") | bold,
                  text(std::to_string(character->busfactor))}),

        }) |
        border;

    // Column 1: Experience bar
    Element experience_gauge = window(
        text("Funding") | bold | center,
        vbox({
            gauge(experience_bar->GetProgress()) | color(Color::Yellow),
            text("Series " +
                 std::to_string(1 + int(experience_bar->GetProgress() * 10)) +
                 " (" +
                 std::to_string(int(experience_bar->GetProgress() * 100)) +
                 "%)") |
                center,
        }));

    // Column 1: Patents

    Element patents =
        vbox({text("Patents") | bold | center, separator(),
              patent_items->Render() | vscroll_indicator | frame}) |
        border | size(HEIGHT, LESS_THAN, 14);

    // Column 2: Quests
    Element quests =
        vbox(
            {text("Quests") | bold | center, separator(),
             text("Main Quest: Druming up the Beats") | color(Color::Yellow),
             gauge(0.7f) | color(Color::Yellow), separator(),
             text("Side Hustle: Comsol Consultancy") | color(Color::GreenLight),
             gauge(0.3f) | color(Color::GreenLight), separator(),
             text("Dev: Quantum Dots") | color(Color::BlueLight),
             gauge(0.5f) | color(Color::BlueLight), separator(),
             text("Active Quests: 3/5") | bold | size(HEIGHT, EQUAL, 1),
             separator(),
             text("Completed Projects: 12") | bold | size(HEIGHT, EQUAL, 1)}) |
        border | size(HEIGHT, EQUAL, 16);

    Element inventory =
        vbox({text("Bees Knees Central") | bold | center, separator(),
              inventory_items->Render() | vscroll_indicator | frame,
              vbox({gauge(encumberance_bar->GetProgress()) | color(Color::Red),
                    text("Selloff (" +
                         std::to_string(
                             int(encumberance_bar->GetProgress() * 100)) +
                         "%)")|center}) |
                  border | size(HEIGHT, EQUAL, 11)}) |
        border | size(HEIGHT, EQUAL, 18);

    // Column 3: Plot Development
    Element plot_dev =
        vbox({
            text("Project Management") | bold | center,
            separator(),
            text("Chapter 1: The Beginning") | color(Color::Green),
            text("✓ Got first Dollar") | color(Color::Green),
            text("✓ Sold Grandmothers Soul to NFT") | color(Color::Green),
            separator(),
            text("Chapter 2: Dark Tidings") | color(Color::Yellow),
            text("✓ Changed Politics") | color(Color::Green),
            text("➤ Confront the cultists") | color(Color::Yellow),
            text("□ Find abandoned warehouse") | color(Color::GrayDark),
            separator(),
            text("Project Progress: Sprint 2 of 5") | bold,
        }) |
        border | size(HEIGHT, EQUAL, 14);

    // Element equipment =
    //     vbox({text("Equipment") | bold | center, separator(),
    //           equipment_items->Render() | vscroll_indicator | frame}) |
    //     border | size(HEIGHT, GREATER_THAN, 28) |yflex;

    Element equipment =
        vbox({
            text("Equipment") | bold | center,
            separator(),
            hbox({text("Weapon: ") | bold | size(WIDTH, EQUAL, 15), text("Vorpal Macbook Pro")}),
            hbox({text("Phone: ") | bold | size(WIDTH, EQUAL, 15), text("Yphone 18")}),
            hbox({text("Backpack: ") | bold | size(WIDTH, EQUAL, 15), text("Alpine Rockjocky")}),
            hbox({text("Biking Helmet: ") | bold | size(WIDTH, EQUAL, 15), text("Specialized")}),
            hbox({text("Headphones: ") | bold | size(WIDTH, EQUAL, 15), text("airbuds")}),
            hbox({text("Bicycle: ") | bold | size(WIDTH, EQUAL, 15), text("Norco Rockfacer XL")}),
            hbox({text("Jacket: ") | bold | size(WIDTH, EQUAL, 15), text("Canadian Goose")}),
            hbox({text("Gloves: ") | bold | size(WIDTH, EQUAL, 15), text("Shitty leather gloves")}),
            hbox({text("Pants: ") | bold | size(WIDTH, EQUAL, 15), text("Shorts")}),
            hbox({text("Shoes: ") | bold | size(WIDTH, EQUAL, 15), text("Campers one size too small")}),
        }) | border | size(HEIGHT, EQUAL, 12);

    // Bottom progress bar
    Element progress_gauge =
        window(
            text("Progress") | bold | center,
            vbox({text("Axing the Monkey " +
                       std::to_string(int(progress_bar->GetProgress() * 100)) +
                       "%"),
                  gauge(progress_bar->GetProgress()) | color(Color::Blue)})) |
        size(HEIGHT, EQUAL, 4);

    return window(countdown_timer->Render(),
                  vbox({
                      // Row 1
                      hbox({
                          vbox({
                              character_stats,
                              experience_gauge,
                              equipment | flex,
                          }) | flex,
                          //Row 2
                          vbox({
                              inventory | flex,
                              patents | flex
                          }) | flex,
                          //Row 3
                          vbox({plot_dev, quests,}) | flex,
                      }),
                      // Progress bar at the bottom
                      progress_gauge,
                      separator(),
                      hbox({back_button->Render(), save_button->Render(),
                            quit_button->Render()}) |
                          center,
                  }));
  });

  return renderer;
}
