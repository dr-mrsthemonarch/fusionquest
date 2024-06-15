#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "banner.h"
// #include "ftxui/component/captured_mouse.hpp"
// #include "ftxui/component/component.hpp"
// #include "ftxui/component/component_base.hpp"
// #include "ftxui/component/screen_interactive.hpp"
// #include "ftxui/dom/elements.hpp"
// #include "ftxui/dom/flexbox_config.hpp"
// #include "ftxui/screen/color.hpp"
//
// using namespace ftxui;
//
// Component Wrap(std::string name, Component component) {
//     return Renderer(component, [name, component] {
//         return hbox({
//             text(name) | size(WIDTH, EQUAL, 15),
//             separator() | color(Color::Default),
//             component->Render() | xflex,
//         });
//     });
// }
//
//
// Function to split the file content into lines and store in a vector
std::vector<std::string> splitLines(const std::string &str) {
    std::vector<std::string> lines;
    std::istringstream stream(str);
    std::string line;

    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    return lines;
}
// auto button_style = ButtonOption::Animated();
// // Definition of the main component. The details are not important.
// Component MainComponent(std::function<void()> show_modal,
//                         std::function<void()> exit) {
//     auto component = Container::Vertical({
//         Button("Show modal", show_modal, button_style),
//         Button("Quit", exit, button_style),
//     });
//     // Polish how the two buttons are rendered:
//     component |= Renderer([&](Element inner) {
//         return vbox({
//                    text("Main component"),
//                    separator(),
//                    inner,
//                }) //
//                | size(WIDTH, GREATER_THAN, 15) //
//                | size(HEIGHT, GREATER_THAN, 15) //
//                | border //
//                | center; //
//     });
//     return component;
// }
//
// int main() {
//     // State of the application:
//     auto screen = ScreenInteractive::TerminalOutput();
//     // There are two layers. One at depth = 0 and the modal window at depth = 1;
//     int depth = 0;
//     std::string rating = "3/5 stars";
//
//
//     const std::string filename = "banner.txt"; // Replace with your file name
//     std::vector<std::string> lines = splitLines(fileContent);
//     auto mainScreen = Renderer([&] {
//         Elements children = {};
//         for (size_t i = std::max(0, (int) lines.size() - 60); i < lines.size(); ++i) {
//             children.push_back(text(lines[i]));
//         }
//         return flexbox(children) | center | color(Color::Yellow) | borderDouble | flex;
//     });
//
//     // At depth=1, The "modal" window.
//     std::vector<std::string> rating_labels = {
//         "1/5 stars", "2/5 stars", "3/5 stars", "4/5 stars", "5/5 stars",
//     };
//     auto on_rating = [&](std::string new_rating) {
//         rating = new_rating;
//         depth = 1;
//     };
//     auto depth_1_container = Container::Horizontal({
//         Button(&rating_labels[0], [&] { on_rating(rating_labels[0]); }),
//         Button(&rating_labels[1], [&] { on_rating(rating_labels[1]); }),
//         Button(&rating_labels[2], [&] { on_rating(rating_labels[2]); }),
//         Button(&rating_labels[3], [&] { on_rating(rating_labels[3]); }),
//         Button(&rating_labels[4], [&] { on_rating(rating_labels[4]); }),
//     });
//
//     auto depth_1_renderer = Renderer(depth_1_container, [&] {
//       return vbox({
//                  text("Do you like Fusion?"),
//                  separator(),
//                  hbox(depth_1_container->Render()),
//              }) | border;
//     });
//     auto main_container = Container::Horizontal(
//           {
//               mainScreen| size(HEIGHT, EQUAL,60) | size(WIDTH,EQUAL,150),
//               depth_1_renderer,
//           },
//           &depth) | size(HEIGHT, EQUAL,60) | size(WIDTH,EQUAL,150);
//
//     auto main_renderer = Renderer(main_container, [&] {
//       Element document = mainScreen->Render();
//
//       if (depth == 1) {
//         document = dbox({
//             document,
//             depth_1_renderer->Render() | clear_under | center,
//         });
//       }
//       return document;
//     });
//
//     screen.Loop(main_renderer);
//     return 0;
// }
// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <memory>  // for allocator, shared_ptr, __shared_ptr_access
#include <string>  // for string, basic_string, char_traits, operator+
#include <vector>  // for vector

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Button, Renderer, Horizontal, Tab
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"

int main() {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
    std::vector<std::string> lines = splitLines(fileContent);
    auto mainScreen = Renderer([&] {
        Elements children = {};
        for (size_t i = std::max(0, (int) lines.size() - 60); i < lines.size(); ++i) {
            children.push_back(text(lines[i]));
        }
    return flexbox(children) |size(HEIGHT,GREATER_THAN,60)| center | color(Color::Yellow) | borderDouble | flex;
    });


  // There are two layers. One at depth = 0 and the modal window at depth = 1;
  int depth = 0;

  // The current rating of FTXUI.
  std::string rating = "3/5 stars";

  // At depth=0, two buttons. One for rating FTXUI and one for quitting.
  auto button_rate_ftxui = Button("Rate FTXUI", [&] { depth = 1; });
  auto button_quit = Button("Quit", screen.ExitLoopClosure());

  auto depth_0_container = Container::Horizontal({
      button_rate_ftxui,
      button_quit,
  });


  // At depth=1, The "modal" window.
  std::vector<std::string> rating_labels = {
      "1/5 stars", "2/5 stars", "3/5 stars", "4/5 stars", "5/5 stars",
  };
  auto on_rating = [&](std::string new_rating) {
    rating = new_rating;
    depth = 0;
  };
  auto depth_1_container = Container::Horizontal({
      Button(&rating_labels[0], [&] { on_rating(rating_labels[0]); }),
      Button(&rating_labels[1], [&] { on_rating(rating_labels[1]); }),
      Button(&rating_labels[2], [&] { on_rating(rating_labels[2]); }),
      Button(&rating_labels[3], [&] { on_rating(rating_labels[3]); }),
      Button(&rating_labels[4], [&] { on_rating(rating_labels[4]); }),
  });

  auto depth_1_renderer = Renderer(depth_1_container, [&] {
    return vbox({
               text("Do you like Fusion?"),
               separator(),
               hbox(depth_1_container->Render()),
           }) |
           border;
  });

  auto main_container = Container::Tab(
      {
          mainScreen,
          depth_1_renderer,
      },
      &depth);

  auto main_renderer = Renderer(main_container, [&] {
    Element document = main_container->Render();

    if (depth == 0) {
      document = dbox({
          document,
          depth_1_renderer->Render() |clear_under| center,
      });
    }
    return document;
  });

  screen.Loop(main_renderer);
  return 0;
}