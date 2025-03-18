//
// Created by Kyle on 16.03.25.
//

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

// Function to generate random stats
std::vector<std::string> GenerateRandomStats() {
    std::vector<std::string> stats;
    stats.push_back("Strength: " + std::to_string(rand() % 18 + 1));    // Random between 1 and 18
    stats.push_back("Dexterity: " + std::to_string(rand() % 18 + 1));   // Random between 1 and 18
    stats.push_back("Intelligence: " + std::to_string(rand() % 18 + 1)); // Random between 1 and 18
    stats.push_back("Wisdom: " + std::to_string(rand() % 18 + 1));       // Random between 1 and 18
    return stats;
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

#endif //FUNCTIONS_H
