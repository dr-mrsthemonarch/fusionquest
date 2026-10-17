#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <vector>
#include <filesystem>

// Function declarations
std::string GetTimestamp();
void Log(const std::string& message);
void InitializeLogFile();
std::vector<std::string> splitLines(const std::string& str);
int CalculateTotal(const std::vector<std::string>& stats);

#endif // FUNCTIONS_H