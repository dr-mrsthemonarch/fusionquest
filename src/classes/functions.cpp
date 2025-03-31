#include "functions.h"
#include <random>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

// Function to get the current timestamp as a string
std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Function to log messages with a timestamp
void Log(const std::string& message) {
    // Open the log file in append mode
    std::ofstream logfile("app_log.txt", std::ios::app);

    if (logfile.is_open()) {
        // Write the timestamp and message to the log file
        logfile << "[" << GetTimestamp() << "] " << message << std::endl;
        logfile.close(); // Close the file after writing
    } else {
        std::cerr << "Error: Unable to open log file!" << std::endl;
    }
}

// Function to initialize the log file (create it if it doesn't exist)
void InitializeLogFile() {
    std::filesystem::path logFilePath("app_log.txt");

    // Check if the log file exists
    if (!std::filesystem::exists(logFilePath)) {
        // Create the log file
        std::ofstream logfile(logFilePath);
        if (logfile.is_open()) {
            logfile << "[" << GetTimestamp() << "] Log file created." << std::endl;
            logfile.close();
        } else {
            std::cerr << "Error: Unable to create log file!" << std::endl;
        }
    }
}

// Function to split the file content into lines and store in a vector
std::vector<std::string> splitLines(const std::string &str) {
    std::vector<std::string> lines;
    std::istringstream stream(str);
    std::string line;

    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    return lines;
}

// Function to calculate the total of stats
int CalculateTotal(const std::vector<std::string>& stats) {
    int total = 0;
    for (const auto& stat : stats) {
        // Extract the number from the stat string (e.g., "Strength: 15" -> 15)
        size_t colon_pos = stat.find(':');
        if (colon_pos != std::string::npos) {
            total += std::stoi(stat.substr(colon_pos + 2)); // +2 to skip ": "
        }
    }
    return total;
}

// Implementation of stat roller function
int RollStat(int range) {
    for (unsigned x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}

float RollChance() {
    // 344 billion = 344 * 10^9 ≈ 2^38.323
    // We need at least 39 bits of randomness
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dist(0, (1ULL << 39) - 1);

    constexpr uint64_t threshold = (1ULL << 39) / 344'000'000'000ULL;

    if (dist(gen) < threshold) {
        static std::uniform_real_distribution<float> small_dist(
            std::nextafter(0.00001f, 1.0f),  // smallest float > 0
            1.0f);                       // largest float < 1
        return small_dist(gen);
    }

    return 1.0f;
}

bool SaveCharacterToFile(const Character& character, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return false;
    }

    // Write character name length first, then the string
    size_t name_length = character.name.length();
    file.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
    file.write(character.name.c_str(), name_length);

    // Write numeric attributes
    file.write(reinterpret_cast<const char*>(&character.race_index), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.class_index), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.fusion_index), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.hustle), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.agility), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.busfactor), sizeof(float));
    file.write(reinterpret_cast<const char*>(&character.techdebt), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.ndas), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.luck), sizeof(int));
    file.write(reinterpret_cast<const char*>(&character.Synergy), sizeof(int));

    file.close();
    return true;
}

Character LoadCharacterFromFile(const std::string& filename) {
    Character character;
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
        return character; // Return default character
    }

    // Read character name length, then the string
    size_t name_length;
    file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));

    char* name_buffer = new char[name_length + 1];
    file.read(name_buffer, name_length);
    name_buffer[name_length] = '\0'; // Null-terminate the string
    character.name = std::string(name_buffer);
    delete[] name_buffer;

    // Read numeric attributes
    file.read(reinterpret_cast<char*>(&character.race_index), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.class_index), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.fusion_index), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.hustle), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.agility), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.busfactor), sizeof(float));
    file.read(reinterpret_cast<char*>(&character.techdebt), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.ndas), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.luck), sizeof(int));
    file.read(reinterpret_cast<char*>(&character.Synergy), sizeof(int));

    file.close();
    return character;
}