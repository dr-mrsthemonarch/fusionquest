//
// Created by drmrsthemonarch on 23.11.25.
//

#include "../include/PatentLibrary.h"
#include "../include/PatentDefinition.h"
#include "../include/Patent.h"
#include <fstream>
#include <iostream>
#include <random>
#include <algorithm>

PatentLibrary::PatentLibrary() {}

bool PatentLibrary::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open named patents config: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;

        if (line == "[PATENT]") {
            PatentDefinition patent;
            if (parsePatentBlock(file, patent)) {
                patents_.push_back(patent);
                std::cout << "Loaded named patent: " << patent.name
                          << " (" << static_cast<int>(patent.type) << ")" << std::endl;
            }
        }
    }

    std::cout << "Loaded " << patents_.size() << " named patents" << std::endl;
    return !patents_.empty();
}

bool PatentLibrary::parsePatentBlock(std::ifstream& file, PatentDefinition& patent) {
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;

        if (line == "[END]") return true;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (key == "name") patent.name = value;
        else if (key == "type") patent.type = stringToPatentType(value);
        else if (key == "power") patent.basePower = std::stoi(value);
        else if (key == "cost") patent.cost = std::stoi(value);
        else if (key == "target") patent.target = stringToPatentTarget(value);
        else if (key == "series_required") patent.seriesRequired = std::stoi(value);
        else if (key == "hustle_mod") patent.hustleModifier = std::stoi(value);
        else if (key == "agility_mod") patent.agilityModifier = std::stoi(value);
        else if (key == "ndas_mod") patent.ndasModifier = std::stoi(value);
        else if (key == "luck_mod") patent.luckModifier = std::stoi(value);
        else if (key == "synergy_mod") patent.synergyModifier = std::stoi(value);
        else if (key == "technical_debt_mod") patent.technicalDebtModifier = std::stoi(value);
        else if (key == "duration") patent.duration = std::stoi(value);
    }

    return false;
}

const PatentDefinition* PatentLibrary::getRandomNamedPatent() const {
    if (patents_.empty()) return nullptr;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, patents_.size() - 1);

    return &patents_[dis(gen)];
}

const PatentDefinition* PatentLibrary::getPatentByIndex(size_t index) const {
    if (index >= patents_.size()) return nullptr;
    return &patents_[index];
}

std::string PatentLibrary::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

PatentType PatentLibrary::stringToPatentType(const std::string& str) {
    if (str == "ATTACK") return PatentType::ATTACK;
    if (str == "HEAL") return PatentType::HEAL;
    if (str == "DEFENSE") return PatentType::DEFENSE;
    if (str == "BUFF") return PatentType::BUFF;
    if (str == "DEBUFF") return PatentType::DEBUFF;
    return PatentType::ATTACK;
}

PatentTarget PatentLibrary::stringToPatentTarget(const std::string& str) {
    if (str == "SELF") return PatentTarget::SELF;
    if (str == "ENEMY") return PatentTarget::ENEMY;
    if (str == "ALLY") return PatentTarget::ALLY;
    return PatentTarget::ENEMY;
}
