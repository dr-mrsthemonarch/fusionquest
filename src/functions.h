#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <vector>
#include <filesystem>
#include "character.h"

// Function declarations
std::string GetTimestamp();
void Log(const std::string& message);
void InitializeLogFile();
std::vector<std::string> splitLines(const std::string& str);
int CalculateTotal(const std::vector<std::string>& stats);
int RollStat(int range);
float RollChance();
bool SaveCharacterToFile(const Character& character, const std::string& filename);
Character LoadCharacterFromFile(const std::string& filename);

#endif // FUNCTIONS_H