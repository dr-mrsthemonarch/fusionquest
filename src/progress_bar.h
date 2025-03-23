// progress_bar.h
#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <iostream>

using namespace ftxui;

class ProgressBar {
public:
    ProgressBar(float update_frequency = 0.01f, float increment = 0.01f, bool loop = false)
        : update_frequency_(update_frequency), increment_(increment), loop_(loop), progress_(0.0f), running_(false) {
        std::cout << "ProgressBar created\n";
    }

    ~ProgressBar() {
        Stop();
        std::cout << "ProgressBar destroyed\n";
    }

    // Delete copy constructor and copy assignment
    ProgressBar(const ProgressBar&) = delete;
    ProgressBar& operator=(const ProgressBar&) = delete;

    // Implement move constructor and move assignment
    ProgressBar(ProgressBar&& other) noexcept
        : update_frequency_(other.update_frequency_),
          increment_(other.increment_),
          loop_(other.loop_),
          progress_(other.progress_.load()),
          running_(other.running_.load()),
          update_thread_(std::move(other.update_thread_)) {
        std::cout << "ProgressBar moved\n";
    }

    ProgressBar& operator=(ProgressBar&& other) noexcept {
        if (this != &other) {
            Stop(); // Stop the current thread
            update_frequency_ = other.update_frequency_;
            increment_ = other.increment_;
            loop_ = other.loop_;
            progress_.store(other.progress_.load());
            running_.store(other.running_.load());
            update_thread_ = std::move(other.update_thread_);
            std::cout << "ProgressBar move-assigned\n";
        }
        return *this;
    }

    // Start the progress bar
    void Start() {
        if (running_) return; // Avoid starting multiple threads
        running_ = true;
        progress_.store(0.0f); // Reset progress to 0
        update_thread_ = std::thread([this] { UpdateLoop(); });
        std::cout << "ProgressBar started\n";
    }

    // Stop the progress bar
    void Stop() {
        if (running_) {
            running_ = false;
            if (update_thread_.joinable()) {
                update_thread_.join();
            }
            std::cout << "ProgressBar stopped\n";
        }
    }

    // Restart the progress bar with new parameters
    void Restart(float update_frequency, float increment, bool loop) {
        Stop(); // Stop the current progress bar
        update_frequency_ = update_frequency;
        increment_ = increment;
        loop_ = loop;
        Start(); // Start with new parameters
        std::cout << "ProgressBar restarted\n";
    }

    // Restart the progress bar from zero
    void RestartFromZero() {
        Stop(); // Stop the current progress bar
        progress_.store(0.0f); // Reset progress to 0
        Start(); // Start again
        std::cout << "ProgressBar restarted from zero\n";
    }

    // Get the current progress value
    float GetProgress() const {
        return progress_.load();
    }

    // Check if the progress bar is running
    bool IsRunning() const {
        return running_.load();
    }

    // Create a renderer for the progress bar
    Component CreateRenderer() {
        return Renderer([this] {
            return hbox({
                text("Progress: "),
                gauge(progress_.load()),
                text(" " + std::to_string(int(progress_.load() * 100)) + "%"),
            });
        });
    }

private:
    // Update loop for the progress bar
    void UpdateLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(int(update_frequency_ * 1000)));
            float new_progress = progress_.load() + increment_;
            if (new_progress > 1.0f) {
                if (loop_) {
                    new_progress = 0.0f; // Restart from zero if looping
                } else {
                    new_progress = 1.0f; // Clamp progress to 1.0
                    running_ = false; // Stop the loop when progress reaches 1.0
                }
            }
            progress_.store(new_progress);
        }
        std::cout << "ProgressBar update loop ended\n";
    }

    float update_frequency_; // Frequency of updates (in seconds)
    float increment_;       // Increment value for progress
    bool loop_;             // Whether to loop the progress bar
    std::atomic<float> progress_; // Current progress value (atomic for thread safety)
    std::atomic<bool> running_;  // Flag to control the update loop
    std::thread update_thread_;  // Thread for updating progress
};

#endif // PROGRESS_BAR_H