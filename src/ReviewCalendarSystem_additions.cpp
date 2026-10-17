// append to ExpandedMonsterTypes.cpp

std::vector<ReviewCalendarSystem::ReviewFlag> ReviewCalendarSystem::exportFlags() const {
    std::vector<ReviewFlag> flags;
    flags.reserve(reviewCalendar_.size());
    for (const auto& sb : reviewCalendar_) {
        flags.push_back({sb.year, sb.month, sb.encountered, sb.defeated});
    }
    return flags;
}

void ReviewCalendarSystem::restoreFlags(const std::vector<ReviewFlag>& flags) {
    for (const auto& fl : flags) {
        for (auto& sb : reviewCalendar_) {
            if (sb.year == fl.year && sb.month == fl.month) {
                sb.encountered = fl.encountered;
                sb.defeated = fl.defeated;
                break;
            }
        }
    }
}
