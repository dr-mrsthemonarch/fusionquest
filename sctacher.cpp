#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <array>
#include <memory>  // for shared_ptr, allocator, __shared_ptr_access
#include "UDPServer.hpp"
#include "Functions.hpp"
#include "TCPServer.hpp"
#include "Commander.hpp"
#include "Parser.hpp"
#include "Responder.hpp"
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

using boost::asio::ip::udp;
using namespace ftxui;
std::atomic<bool> batman(false); // Use atomic for thread-safe boolean
std::atomic<bool> joker(false); // Use atomic for thread-safe boolean

// Function to control the UDPserver
std::string udp_server(boost::asio::io_context& io_context,
                       std::unique_ptr<std::thread>& io_thread,
                       std::unique_ptr<UDPServer>& server,
                       std::string command,
                       SharedVector& sharedVec) {
    std::string returner;

    if (command == "start") {
        if (!batman.load()) {
            batman.store(true);
            // Create a new server instance and run it in a separate thread
            server = std::make_unique<UDPServer>(io_context, 3040,sharedVec);

            io_thread = std::make_unique<std::thread>([&io_context]() {
                io_context.run();
            });
            returner = "Starting UDP Discovery...";
        }
    } else if (command == "stop") {
        if (batman.load()) {
            batman.store(false);
            // Stop the io_context and wait for the thread to finish
            io_context.stop();
            if (io_thread && io_thread->joinable()) {
                io_thread->join();
            }

            // Reset io_context and delete the server instance
            io_context.restart();
            server.reset();
            //return a response
            returner = "Server stopped.";
        } else {
            returner = "Server is not running.";
        }
    }else if (command == "exit") {
        if (batman.load()) {
            batman.store(false);
            io_context.stop();
            if (io_thread && io_thread->joinable()) {
                io_thread->join();
            }
            io_context.restart();
            server.reset();
        }
        returner = "Exiting...";
    }
    return returner;
}

// Function to control the TCPserver
std::string tcp_control(boost::asio::io_context& io_context,
                        std::unique_ptr<std::thread>& io_thread,
                        std::unique_ptr<TCPServer>& server,
                        const std::string& command,Commander& cmd,
                        SharedVector& sharedVec,
                        SharedVector& clientVec) {
    std::string returner;

    if (command == "start") {
        if (!joker.load()) {
            joker.store(true);
            // Create a new TCP server instance and run it in a separate thread
            server = std::make_unique<TCPServer>(io_context, 1234, cmd, sharedVec,clientVec);

            io_thread = std::make_unique<std::thread>([&io_context]() {
                io_context.run();
            });
            returner = "Starting TCP server...";
        }
    } else if (command == "stop") {
        if (joker.load()) {
            joker.store(false);
            // Stop the io_context and wait for the thread to finish
            io_context.stop();
            if (io_thread && io_thread->joinable()) {
                io_thread->join();
            }

            // Reset io_context and delete the server instance
            io_context.restart();
            server.reset();

            returner = "TCP Server stopped.";
        } else {
            returner = "TCP Server is not running.";
        }
    }else if (command == "exit") {
        if (joker.load()) {
            joker.store(false);
            io_context.stop();
            if (io_thread && io_thread->joinable()) {
                io_thread->join();
            }
            io_context.restart();
            server.reset();
        }
        returner = "Exiting...";
    }

    return returner;
}

int main() {
    auto screen = ScreenInteractive::Fullscreen();
    //things needed to start and stop the UDPServer
    SharedVector sharedVec;
    SharedVector clientVec;
    boost::asio::io_context udpcontext;
    boost::asio::io_context tcpcontext;
    boost::asio::io_service io_service;
    bool running = true;

    std::unique_ptr<UDPServer> udpserver;
    std::unique_ptr<TCPServer> tcpserver;
    std::unique_ptr<std::thread> udpthread;
    std::unique_ptr<std::thread> tcpthread;
    std::vector<std::string> cli_entries;
    Commander commander(sharedVec);

    // Add commands and their responses
    commander.addCommand("++addr", "14", 2);
    commander.addCommand("++reset","",0);
    commander.addCommand("++ver", "1.3.4", 0);
    commander.addCommand("++mode", "0", 1);
    commander.addCommand("++auto", "0", 1);
    commander.addCommand("++eoi", "0", 1);
    commander.addCommand("++eos", "0", 1);
    commander.addCommand("++eot_enable", "0", 1);
    commander.addCommand("++eot_char", "0", 1);
    commander.addCommand("++ifc", "Conntroller in Charge", 0);
    commander.addCommand("++llo", "Display Off", 0);
    commander.addCommand("++loc", "Display On", 0);
    commander.addCommand("++lon", "0", 1);
    commander.addCommand("++read_tmo_ms", "1", 1);
    commander.addCommand("++srq", "0", 1);
    commander.addCommand("++status", "0", 1);
    // ----------------------------------------------------------------------
    auto udpStart = [&] {
        std::lock_guard<std::mutex> lock(sharedVec.vecMutex);
        sharedVec.vec.push_back(udp_server(udpcontext, udpthread, udpserver,"start",sharedVec));
    };

    auto udpStop = [&] {
        std::lock_guard<std::mutex> lock(sharedVec.vecMutex);
        sharedVec.vec.push_back(udp_server(udpcontext, udpthread, udpserver,"stop",sharedVec));
    };
    auto tcpStart = [&] {
        std::lock_guard<std::mutex> lock(sharedVec.vecMutex);
        sharedVec.vec.push_back(tcp_control(tcpcontext, tcpthread, tcpserver,"start",commander,sharedVec,clientVec));
    };

    auto tcpStop = [&] {
        std::lock_guard<std::mutex> lock(sharedVec.vecMutex);
        sharedVec.vec.push_back(tcp_control(tcpcontext, tcpthread, tcpserver,"stop",commander,sharedVec,clientVec));
    };
    auto Exit = [&] {
        std::lock_guard<std::mutex> lock(sharedVec.vecMutex);
        sharedVec.vec.push_back(udp_server(udpcontext, udpthread, udpserver,"exit",sharedVec));
        sharedVec.vec.push_back(tcp_control(tcpcontext, tcpthread, tcpserver,"exit",commander,sharedVec,clientVec));
        running = false;
        screen.Exit();
    };

    // -- client list
    // ----------------------------------------------------------------------
    std::array<bool, 10> states;

    auto checkboxes = Container::Vertical({});
    for (int i = 0; i < clientVec.vec.size(); ++i) {
        states[i] = false;
        checkboxes->Add(Checkbox(clientVec.vec[i],
                                 &states[i]) );
    }
    Component checkframe = Renderer(checkboxes, [&] {
        return checkboxes->Render() | vscroll_indicator | frame |
        size(HEIGHT, LESS_THAN, 5) | border | color(Color::Default);
    });

    checkframe=Wrap("To be Done",checkframe);


    std::vector<std::string> input_entries;
    auto input_option = InputOption();
    std::string input_add_content;
    //empty the input after hitting enter.
    input_option.on_enter = [&] {
        input_entries.push_back(input_add_content);
        input_add_content = "";
    };
    Component input_add = Input(&input_add_content, "input files", input_option);

    // -- CLI -----------------------------------------------------------------


    auto cli_options = InputOption();
    std::string cli_add_content;
    cli_options.on_enter = [&] {
        sharedVec.vec.push_back(cli_add_content);
        cli_add_content = "";
    };
    Component cli_add = Input(&cli_add_content,"commands...",cli_options);


    auto cliArea = Renderer(cli_add,[&] {
        return window(text("cli input"),vbox(cli_add->Render())) | color(Color::Purple4);
    }) | size(HEIGHT, EQUAL, 1);

    // -- UDP Buttons -----------------------------------------------------------------

    auto udpbuttons = Container::Horizontal({
        Button("Start", udpStart) | center | color(Color::Green), // turn udp on
        Button("Stop", udpStop) | center | color(Color::Red), // turn udp off
    }) | size(HEIGHT, EQUAL, 3);
    udpbuttons = Wrap("UDP Discovery",udpbuttons);
    // -- TCP Buttons -----------------------------------------------------------------

    auto tcpbuttons = Container::Horizontal({
        Button("Start", tcpStart) | center | color(Color::Green), // turn udp on
        Button("Stop", tcpStop) | center | color(Color::Red), // turn udp off
        Button("Exit", Exit) | center | color(Color::DarkRed),
    }) | size(HEIGHT, EQUAL, 5) ;
    tcpbuttons = Wrap("TCP Server",tcpbuttons);

    // -- Output -----------------------------------------------------------------

    auto output = Renderer([&] {
        Elements children = {
            text("Server status, connections and outputs are printed here."),
            separator() | color(Color::Yellow),
        };
        for (size_t i = std::max(0, (int)sharedVec.vec.size() - 11); i < sharedVec.vec.size(); ++i) {
            std::lock_guard<std::mutex> lock(sharedVec.vecMutex);
            children.push_back(text(sharedVec.vec[i]));
        }
        return window(text("Output"), vbox(children) | color(Color::Yellow4) | flex ) | color(Color::Yellow4);
    });


    // -- Layout -----------------------------------------------------------------
    auto layout = Container::Vertical({
        checkframe,
        input_add,
        udpbuttons,
        tcpbuttons
    }) | size(HEIGHT, EQUAL, 43);

    auto component = Renderer(layout, [&] {
        return vbox({
            checkframe->Render(),
            separator() | color(Color::Default),
            udpbuttons->Render(),
            separator() | color(Color::Default),
            tcpbuttons->Render()
        }) | size(WIDTH, GREATER_THAN, 30) | borderStyled(ROUNDED,Color::Default);

    });

    auto topwindows = Container::Horizontal({
        component,
        output | size(HEIGHT, EQUAL, 15) | yflex,
    });

    auto layoutmain = Container::Vertical({
        topwindows ,
        cliArea | flex,
    });


    auto renderer = Renderer(layoutmain, [&] {
        return layoutmain->Render() ;
    }); //main render for the entire interface.

    //create a new thread to run the output continously/force redraw. Required for the output window to update on it's own instead of waiting for an event.

    auto screenRedraw = std::thread([&](){
        while(running){
            screen.PostEvent(ftxui::Event::Custom);
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Prevent High CPU Usage.
        }
    });

    screen.Loop(renderer);

    screenRedraw.join();
}


// // Copyright 2020 Arthur Sonzogni. All rights reserved.
// // Use of this source code is governed by the MIT license that can be found in
// // the LICENSE file.
// #include <cstddef>  // for size_t
// #include <memory>    // for shared_ptr, __shared_ptr_access, allocator
// #include <string>  // for string, basic_string, to_string, operator+, char_traits
// #include <vector>  // for vector
// #include <iostream>
// #include <sstream>
// #include "banner.h"
// #include "ftxui/component/captured_mouse.hpp"  // for ftxui
// #include "ftxui/component/component.hpp"  // for Radiobox, Vertical, Checkbox, Horizontal, Renderer, ResizableSplitBottom, ResizableSplitRight
// #include "ftxui/component/component_base.hpp"      // for ComponentBase
// #include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
// #include "ftxui/dom/elements.hpp"  // for text, window, operator|, vbox, hbox, Element, flexbox, bgcolor, filler, flex, size, border, hcenter, color, EQUAL, bold, dim, notflex, xflex_grow, yflex_grow, HEIGHT, WIDTH
// #include "ftxui/dom/flexbox_config.hpp"  // for FlexboxConfig, FlexboxConfig::AlignContent, FlexboxConfig::JustifyContent, FlexboxConfig::AlignContent::Center, FlexboxConfig::AlignItems, FlexboxConfig::Direction, FlexboxConfig::JustifyContent::Center, FlexboxConfig::Wrap
// #include "ftxui/screen/color.hpp"        // for Color, Color::Black
//
// using namespace ftxui;
//
// Component Wrap(std::string name, Component component) {
//   return Renderer(component, [name, component] {
//       return hbox({
//           text(name) | size(WIDTH, EQUAL, 15),
//           separator() |  color(Color::Default),
//           component->Render() | xflex ,
//       });
//   });
// }
//
//
//
// // Function to split the file content into lines and store in a vector
// std::vector<std::string> splitLines(const std::string& str) {
//     std::vector<std::string> lines;
//     std::istringstream stream(str);
//     std::string line;
//
//     while (std::getline(stream, line)) {
//         lines.push_back(line);
//     }
//
//     return lines;
// }
//
// int main() {
//   auto screen = ScreenInteractive::Fullscreen();
//   const std::string filename = "banner.txt"; // Replace with your file name
//     std::vector<std::string> lines = splitLines(fileContent);
//     auto mainScreen = Renderer([&] {
//         Elements children = {
//         paragraph(lines[1]),};
//         for (size_t i = std::max(0, (int)lines.size() - 60); i < lines.size(); ++i) {
//             children.push_back(text(lines[i]));
//         }
//         return flexbox(children) | center | color(Color::Yellow) |borderDouble| flex;
//     });
//     screen.Loop(mainScreen);
// }

int main() {
  using namespace ftxui;
  auto screen = ScreenInteractive::TerminalOutput();

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
  auto depth_0_renderer = Renderer(depth_0_container, [&] {
    return vbox({
               text("Modal dialog example"),
               separator(),
               text("☆☆☆ FTXUI:" + rating + " ☆☆☆") | bold,
               filler(),
               hbox({
                   button_rate_ftxui->Render(),
                   filler(),
                   button_quit->Render(),
               }),
           }) |
           border | size(HEIGHT, GREATER_THAN, 18) | center;
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
               text("Do you like FTXUI?"),
               separator(),
               hbox(depth_1_container->Render()),
           }) |
           border;
  });

  auto main_container = Container::Tab(
      {
          depth_0_renderer,
          depth_1_renderer,
      },
      &depth);

  auto main_renderer = Renderer(main_container, [&] {
    Element document = depth_0_renderer->Render();

    if (depth == 1) {
      document = dbox({
          document,
          depth_1_renderer->Render() | clear_under | center,
      });
    }
    return document;
  });

  screen.Loop(main_renderer);
  return 0;
}

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
===
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