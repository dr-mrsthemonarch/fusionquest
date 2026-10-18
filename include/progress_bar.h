// progress_bar.h
#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <algorithm>

using namespace ftxui;

// target-driven bar. engine/UI calls SetTarget(0..1) each frame; background thread
// interpolates progress_ toward target by increment_ every update_frequency_ seconds.
class ProgressBar {
public:
    ProgressBar(float update_frequency = 0.05f, float increment = 0.02f)
        : update_frequency_(update_frequency), increment_(increment),
          progress_(0.0f), target_(0.0f), running_(false) {}

    ~ProgressBar() { Stop(); }

    ProgressBar(const ProgressBar&) = delete;
    ProgressBar& operator=(const ProgressBar&) = delete;

    void Start() {
        if (running_) return;
        running_ = true;
        update_thread_ = std::thread([this] { UpdateLoop(); });
    }

    void Stop() {
        running_ = false;
        if (update_thread_.joinable()) update_thread_.join();
    }

    void SetTarget(float target) { target_.store(std::clamp(target, 0.0f, 1.0f)); }

    // jump immediately (e.g. after load / series reset), no animation
    void SetImmediate(float value) {
        float v = std::clamp(value, 0.0f, 1.0f);
        target_.store(v);
        progress_.store(v);
    }

    float GetProgress() const { return progress_.load(); }
    float GetTarget() const { return target_.load(); }
    bool IsRunning() const { return running_.load(); }

private:
    void UpdateLoop() {
        while (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(int(update_frequency_ * 1000)));
            float cur = progress_.load(), tgt = target_.load();
            if (cur < tgt) cur = std::min(tgt, cur + increment_);
            else if (cur > tgt) cur = std::max(tgt, cur - increment_);
            progress_.store(cur);
        }
    }

    float update_frequency_; // seconds between steps
    float increment_;        // max step per tick
    std::atomic<float> progress_;
    std::atomic<float> target_;
    std::atomic<bool> running_;
    std::thread update_thread_;
};

#endif // PROGRESS_BAR_H