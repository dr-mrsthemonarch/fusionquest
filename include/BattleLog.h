#ifndef BATTLELOG_H
#define BATTLELOG_H

#include <string>
#include <deque>

// replaces Qt updateBattleLog(QString). plain lines, capped.
// optional mirror: every line added here is also added to the mirror log. engine uses
// this so the full "detail log" contains summary lines AND captured stdout details.
class BattleLog {
public:
    explicit BattleLog(size_t cap = 500) : cap_(cap) {}

    void add(const std::string& line) {
        lines_.push_back(line);
        total_++;
        if (lines_.size() > cap_) lines_.pop_front();
        if (mirror_) mirror_->add(line);
    }

    void setMirror(BattleLog* mirror) { mirror_ = mirror; }
    const std::deque<std::string>& lines() const { return lines_; }
    void clear() { lines_.clear(); }
    // monotonic count of every line ever added (survives cap + clear). lets a reader
    // work out how many NEW lines arrived since it last looked.
    size_t totalAdded() const { return total_; }

private:
    std::deque<std::string> lines_;
    size_t cap_;
    size_t total_ = 0;
    BattleLog* mirror_ = nullptr;
};

#endif