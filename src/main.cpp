#include "progress_bar.h"
#include "logger.h"
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <thread>
#include <chrono>
#include <iostream>

using namespace ftxui;

int main(int argc, char* argv[]) {
    // Parse command-line arguments
    LogLevel log_level = LogLevel::INFO; // Default log level
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v") {
            log_level = LogLevel::DEBUG; // Verbose mode (DEBUG)
        } else if (arg == "-i") {
            log_level = LogLevel::INFO; // Info mode (INFO)
        } else if (arg == "-e") {
            log_level = LogLevel::ERROR; // Error mode (ERROR)
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        }
    }

    // Create a logger
    Logger logger;
    logger.SetLogLevel(log_level); // Set the log level based on the command-line option

    // Check if the log file is open
    if (!logger.IsLogFileOpen()) {
        std::cerr << "Failed to open log file. Exiting." << std::endl;
        return 1;
    }

    auto screen = ScreenInteractive::Fullscreen();

    // Create a progress bar
    ProgressBar progress_bar(0.01f, 0.01f, false); // Non-looping by default

    // Buttons to control the progress bar
    auto start_button = Button("Start", [&progress_bar, &logger] {
        try {
            progress_bar.Start();
            logger.Log("Start button clicked", LogLevel::INFO);
        } catch (const std::exception& e) {
            logger.Log("Error in Start button: " + std::string(e.what()), LogLevel::ERROR);
        }
    });

    auto stop_button = Button("Stop", [&progress_bar, &logger] {
        try {
            progress_bar.Stop();
            logger.Log("Stop button clicked", LogLevel::INFO);
        } catch (const std::exception& e) {
            logger.Log("Error in Stop button: " + std::string(e.what()), LogLevel::ERROR);
        }
    });

    auto restart_button = Button("Restart", [&progress_bar, &logger] {
        try {
            progress_bar.RestartFromZero();
            logger.Log("Restart button clicked", LogLevel::INFO);
        } catch (const std::exception& e) {
            logger.Log("Error in Restart button: " + std::string(e.what()), LogLevel::ERROR);
        }
    });

    auto loop_button = Button("Toggle Loop", [&progress_bar, &logger] {
        try {
            progress_bar.Restart(0.01f, 0.01f, !progress_bar.IsRunning());
            logger.Log("Toggle Loop button clicked", LogLevel::INFO);
        } catch (const std::exception& e) {
            logger.Log("Error in Toggle Loop button: " + std::string(e.what()), LogLevel::ERROR);
        }
    });

    // Layout
    auto layout = Container::Vertical({
        start_button,
        stop_button,
        restart_button,
        loop_button,
    });

    // Renderer
    auto renderer = Renderer(layout, [&progress_bar, start_button, stop_button, restart_button, loop_button] {
        return vbox({
            progress_bar.CreateRenderer()->Render(),
            separator(),
            hbox({
                start_button->Render(),
                stop_button->Render(),
                restart_button->Render(),
                loop_button->Render(),
            }) | center,
        });
    });

    // Start a refresh thread to post events periodically
    std::thread refresh_thread;
    try {
        refresh_thread = std::thread([&screen, &logger] {
            while (true) {
                try {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Refresh every 10ms
                    screen.PostEvent(ftxui::Event::Custom); // Trigger a re-render
                    logger.Log("Refresh thread posted Event::Custom", LogLevel::DEBUG); // Log as DEBUG
                } catch (const std::exception& e) {
                    logger.Log("Error in refresh thread: " + std::string(e.what()), LogLevel::ERROR);
                }
            }
        });
    } catch (const std::exception& e) {
        logger.Log("Failed to start refresh thread: " + std::string(e.what()), LogLevel::ERROR);
        return 1;
    }

    // Detach the refresh thread to avoid blocking the main thread
    refresh_thread.detach();

    // Run the FTXUI loop
    try {
        screen.Loop(renderer);
    } catch (const std::exception& e) {
        logger.Log("Error in FTXUI loop: " + std::string(e.what()), LogLevel::ERROR);
        return 1;
    }

    return 0;
}