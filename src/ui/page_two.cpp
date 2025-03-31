// page_two.cpp
#include "ui.h"
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

// Component CreateLoadPage(int* selected_page) {
//     auto page_two = Container::Vertical({
//         Button("Back to Main", [selected_page] { *selected_page = MAIN_MENU; }),
//     });
//
//     auto renderer = Renderer(page_two, [page_two] {
//         return vbox({
//             text("Page 2") | bold | center,
//             separator(),
//             text("You've reached Page 2.\nExplore the content here.") | center | color(Color::Green),
//             page_two->Render(),
//         });
//     });
//
//     return renderer;
// }