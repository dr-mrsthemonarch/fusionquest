#ifndef GAMECLOCK_H
#define GAMECLOCK_H

#include <chrono>
#include <string>
#include <ftxui/component/component_base.hpp>
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
        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(target_time_ - now);

        if (remaining.count() <= 0) return "Fusion Quest - LAUNCH TIME!";

        auto seconds = remaining.count();
        auto minutes = seconds / 60; seconds %= 60;
        auto hours = minutes / 60; minutes %= 60;
        auto days = hours / 24; hours %= 24;
        auto months = days / 30; days %= 30;
        auto years = months / 12; months %= 12;

        return "Fusion Quest - Power Plant Comissioning: " + std::to_string(years) +
               "y " + std::to_string(months) + "m " + std::to_string(days) + "d " +
               std::to_string(hours) + "h " + std::to_string(minutes) + "m " +
               std::to_string(seconds) + "s";
    }

    // NEW: called on new-game / load-game so clock restarts at 0. same instance kept alive
    // everywhere (stats_page render, main.cpp tick loop) - just rewinds start_time_.
    void ResetClock() {
        start_time_ = std::chrono::system_clock::now();
        target_time_ = start_time_ + std::chrono::hours(24 * 365 * 20);
    }

    // NEW: feeds GameEngine::tick(). elapsed seconds since this component was constructed.
    [[nodiscard]] double GetElapsedSeconds() const {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double>>(now - start_time_).count();
    }

    Element Render() {
        if (animation_frame_++ % 60 == 0) { /* refresh tick, screen auto-redraws */ }
        return text(GetCountdownString()) | bold | center;
    }

private:
    std::chrono::system_clock::time_point start_time_;
    std::chrono::system_clock::time_point target_time_;
    int animation_frame_ = 0;
};

#endif
