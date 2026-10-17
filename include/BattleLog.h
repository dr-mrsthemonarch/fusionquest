#ifndef BATTLELOG_H
#define BATTLELOG_H

#include <string>
#include <vector>
#include <deque>

// replaces Qt updateBattleLog(QString). plain lines, capped, ftxui renders last N via vscroll.
class BattleLog {
public:
    explicit BattleLog(size_t cap = 500) : cap_(cap) {}

    void add(const std::string& line) {
        lines_.push_back(line);
        if (lines_.size() > cap_) lines_.pop_front();
    }

    const std::deque<std::string>& lines() const { return lines_; }
    void clear() { lines_.clear(); }

private:
    std::deque<std::string> lines_;
    size_t cap_;
};

#endif
