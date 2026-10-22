#ifndef ITEM_GENERATOR_H
#define ITEM_GENERATOR_H
#include <string>
#include <vector>
#include <map>

struct GeneratedItem {
    std::string name;
    int value;
};
class ItemGenerator {
private:
    std::vector<std::string> techAdjectives;
    std::vector<std::string> officeNouns;
    std::vector<std::string> scienceNouns;
    std::vector<std::string> fancySuffixes;
    std::vector<std::string> mythicalNouns;
    std::vector<std::string> itemTypes;
    std::vector<std::string> companySuffixes;
    std::map<std::string, int> valueModifiers;
    
    bool loadedSuccessfully;
    std::vector<std::string> collectKeywords_;
    double collectBias_ = 0.0;

public:
    ItemGenerator();
    bool loadWordLists(const std::string& configPath = "config/items/");
    void generateRandomItem(class Character* character);
    GeneratedItem generateRandomItem();
    void reloadWordLists();
    // quest bias: with chance p a generated item name uses one of these keywords
    void setCollectBias(const std::vector<std::string>& kws, double chance) { collectKeywords_ = kws; collectBias_ = chance; }
private:
    bool loadListFromFile(const std::string& filename, std::vector<std::string>& targetList);
    bool loadValueModifiers(const std::string& filename);
    std::string generateCommonName();
    std::string generateMythicalName();
    std::string generateEpicName();
    std::string generateCorporateName();
    int calculateItemValue(const std::string& itemName, int nameStyle);
};

#endif