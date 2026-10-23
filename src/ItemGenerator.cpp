// SPG Engine made by drmrsthemonarch
#include "../include/ConfigStore.h"
#include "../include/ItemGenerator.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../include/Character.h"


ItemGenerator::ItemGenerator() : loadedSuccessfully(false) {
    std::srand(std::time(0));
}

int ItemGenerator::calculateItemValue(const std::string &itemName, int nameStyle) {
    int baseValue = rand() % 15 + 1; // Base value 1-15
    int bonusValue = 0;

    // Style-based bonuses
    if (nameStyle >= 50) bonusValue += 10; // Fancy names
    if (nameStyle >= 80) bonusValue += 15; // Epic names
    if (nameStyle >= 95) bonusValue += 20; // Corporate names

    // Keyword-based value modifiers
    for (const auto &modifier: valueModifiers) {
        if (itemName.find(modifier.first) != std::string::npos) {
            bonusValue += modifier.second;
        }
    }

    return baseValue + bonusValue;
}

void ItemGenerator::generateRandomItem(Character *character) {
    //needs to later have time functionality or rather legendary items only generate after 19 years of play.
    if (!loadedSuccessfully) {
        std::cerr << "Word lists not loaded properly. Cannot generate item." << std::endl;
        return;
    }

    std::string itemName;
    int nameStyle = rand() % 100; // 0-99 for different naming styles

    if (nameStyle < 50) {
        itemName = generateCommonName(); // 50% chance all years of play
    } else if (nameStyle < 80) {
        itemName = generateMythicalName(); // 30% chance only after 14 years of play
    } else if (nameStyle < 95) {
        itemName = generateEpicName(); // 15% chance only after 17 years of play
    } else {
        itemName = generateCorporateName(); // 5% chance only after 19 years of play
    }

    // Fallback if something went wrong with name generation
    if (itemName.empty()) {
        itemName = "Generic Tech Item";
    }

    int totalValue = calculateItemValue(itemName, nameStyle);
    character->addInventoryItem(itemName, totalValue);

    std::cout << "Generated: " << itemName << " (Value: " << totalValue << ")" << std::endl;
}

GeneratedItem ItemGenerator::generateRandomItem() {
    GeneratedItem result;

    if (!loadedSuccessfully) {
        std::cerr << "Word lists not loaded properly. Cannot generate item." << std::endl;
        result.name = "Generic Tech Item";
        result.value = 1;
        return result;
    }

    std::string itemName;
    int nameStyle = rand() % 100; // 0-99 for naming style

    if (nameStyle < 50) {
        itemName = generateCommonName();
    } else if (nameStyle < 80) {
        itemName = generateMythicalName();
    } else if (nameStyle < 95) {
        itemName = generateEpicName();
    } else {
        itemName = generateCorporateName();
    }

    if (itemName.empty()) {
        itemName = "Generic Tech Item";
    }

    int totalValue = calculateItemValue(itemName, nameStyle);

    if (!collectKeywords_.empty() && !techAdjectives.empty() && (rand() % 10000) < collectBias_ * 10000) {
        itemName = techAdjectives[rand() % techAdjectives.size()] + " " + collectKeywords_[rand() % collectKeywords_.size()];
        totalValue = calculateItemValue(itemName, 0);
    }

    result.name = itemName;
    result.value = totalValue;

    return result;
}

std::string ItemGenerator::generateCommonName() {
    int adjIndex = rand() % techAdjectives.size();

    // Choose between office item or science concept
    std::vector<std::string> *nounList = (rand() % 2 == 0) ? &officeNouns : &scienceNouns;
    int nounIndex = rand() % nounList->size();

    return techAdjectives[adjIndex] + " " + (*nounList)[nounIndex];
}

std::string ItemGenerator::generateMythicalName() {
    int itemIndex = rand() % itemTypes.size();
    int mythIndex = rand() % mythicalNouns.size();

    return itemTypes[itemIndex] + " of " + mythicalNouns[mythIndex];
}

std::string ItemGenerator::generateEpicName() {
    int adjIndex1 = rand() % techAdjectives.size();
    int adjIndex2 = rand() % techAdjectives.size();

    // Check for duplicate adjectives and regenerate if needed
    while (adjIndex1 == adjIndex2) {
        adjIndex2 = rand() % techAdjectives.size();
    }

    int nounIndex = rand() % officeNouns.size();
    int prefixIndex = rand() % fancySuffixes.size();

    return techAdjectives[adjIndex1] + " " + techAdjectives[adjIndex2] + " " + officeNouns[nounIndex] + " " + fancySuffixes[prefixIndex];
}

std::string ItemGenerator::generateCorporateName() {
    int adjIndex = rand() % techAdjectives.size();
    int nounIndex = rand() % scienceNouns.size();
    int suffixIndex = rand() % companySuffixes.size();

    return techAdjectives[adjIndex] + " " + scienceNouns[nounIndex] + " " + companySuffixes[suffixIndex];
}

bool ItemGenerator::loadWordLists(const std::string &configPath) {
    bool success = true;

    success &= loadListFromFile(configPath + "adjectives.txt", techAdjectives);
    success &= loadListFromFile(configPath + "office_nouns.txt", officeNouns);
    success &= loadListFromFile(configPath + "science_nouns.txt", scienceNouns);
    success &= loadListFromFile(configPath + "fancy_suffix.txt", fancySuffixes);
    success &= loadListFromFile(configPath + "mythical_nouns.txt", mythicalNouns);
    success &= loadListFromFile(configPath + "it_words.txt", itemTypes);
    success &= loadListFromFile(configPath + "company_suffixes.txt", companySuffixes);
    success &= loadValueModifiers(configPath + "value_modifiers.txt");

    loadedSuccessfully = success;

    if (!success) {
        std::cerr << "Warning: Some word lists failed to load. Item generation may be limited." << std::endl;
    }

    return success;
}

bool ItemGenerator::loadListFromFile(const std::string &filename, std::vector<std::string> &targetList) {
    ConfigFile file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open file: " << filename << std::endl;
        return false;
    }

    targetList.clear();
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (!line.empty() && line[0] != '#') {
            targetList.push_back(line);
        }
    }

    file.close();
    std::cout << "Loaded " << targetList.size() << " items from " << filename << std::endl;
    return !targetList.empty();
}

bool ItemGenerator::loadValueModifiers(const std::string &filename) {
    ConfigFile file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open value modifiers file: " << filename << std::endl;
        return false;
    }

    valueModifiers.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            std::istringstream iss(line);
            std::string word;
            int value;
            if (iss >> word >> value) {
                valueModifiers[word] = value;
            }
        }
    }

    file.close();
    return true;
}

void ItemGenerator::reloadWordLists() {
    loadedSuccessfully = false;
    loadWordLists();
}
