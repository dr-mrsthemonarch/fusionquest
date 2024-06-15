// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <cstddef>  // for size_t
#include <memory>    // for shared_ptr, __shared_ptr_access, allocator
#include <string>  // for string, basic_string, to_string, operator+, char_traits
#include <vector>  // for vector
#include <iostream>
#include <fstream>
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Radiobox, Vertical, Checkbox, Horizontal, Renderer, ResizableSplitBottom, ResizableSplitRight
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, window, operator|, vbox, hbox, Element, flexbox, bgcolor, filler, flex, size, border, hcenter, color, EQUAL, bold, dim, notflex, xflex_grow, yflex_grow, HEIGHT, WIDTH
#include "ftxui/dom/flexbox_config.hpp"  // for FlexboxConfig, FlexboxConfig::AlignContent, FlexboxConfig::JustifyContent, FlexboxConfig::AlignContent::Center, FlexboxConfig::AlignItems, FlexboxConfig::Direction, FlexboxConfig::JustifyContent::Center, FlexboxConfig::Wrap
#include "ftxui/screen/color.hpp"        // for Color, Color::Black

using namespace ftxui;

Component Wrap(std::string name, Component component) {
  return Renderer(component, [name, component] {
      return hbox({
          text(name) | size(WIDTH, EQUAL, 15),
          separator() |  color(Color::Default),
          component->Render() | xflex ,
      });
  });
}



// Function to read a file and store each line in a vector
std::vector<std::string> readFileToVector(const std::string& filename) {
    std::vector<std::string> lines;
    std::ifstream file(filename);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }

    return lines;
}

int main() {
  auto screen = ScreenInteractive::Fullscreen();
    std::string filename = "banner.txt"; // Replace with your file name
    std::vector<std::string> lines = readFileToVector(filename);
    auto mainScreen = Renderer([&] {
        Elements children = {
        paragraph(lines[1]),};
        return flexbox(children) | center | color(Color::Yellow) |borderDouble| flex;
    });
    screen.Loop(mainScreen);
}