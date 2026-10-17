#include "../include/functions.h"
#include <random>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// REMOVED: SaveCharacterToFile/LoadCharacterFromFile - depended on old placeholder
// `character.h` struct (deleted, see rename decision). superseded by SaveGame (versioned
// binary, covers full SPG Character + engine progress). RollStat/RollChance also removed -
// were used by fusionquest's placeholder stat-rolling, not applicable to SPG Character model
// (fixed base stats + equipment, see character_creation.cpp note).

std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Log(const std::string& message) {
    std::ofstream logfile("app_log.txt", std::ios::app);
    if (logfile.is_open()) {
        logfile << "[" << GetTimestamp() << "] " << message << std::endl;
        logfile.close();
    } else {
        std::cerr << "Error: Unable to open log file!" << std::endl;
    }
}

void InitializeLogFile() {
    std::filesystem::path logFilePath("app_log.txt");
    if (!std::filesystem::exists(logFilePath)) {
        std::ofstream logfile(logFilePath);
        if (logfile.is_open()) {
            logfile << "[" << GetTimestamp() << "] Log file created." << std::endl;
            logfile.close();
        } else {
            std::cerr << "Error: Unable to create log file!" << std::endl;
        }
    }
}

std::vector<std::string> splitLines(const std::string& str) {
    std::vector<std::string> lines;
    std::istringstream stream(str);
    std::string line;
    while (std::getline(stream, line)) lines.push_back(line);
    return lines;
}

int CalculateTotal(const std::vector<std::string>& stats) {
    int total = 0;
    for (const auto& stat : stats) {
        size_t colon_pos = stat.find(':');
        if (colon_pos != std::string::npos) total += std::stoi(stat.substr(colon_pos + 2));
    }
    return total;
}
