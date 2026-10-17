#ifndef NEMESISSYSTEM_H
#define NEMESISSYSTEM_H

#include <string>
#include <vector>
#include "Monster.h"

enum class NemesisType {
    ALPHA,      // Year 1-5: The Original Rival
    BETA,       // Year 6-10: The Evolved Threat
    OMEGA,      // Year 11-15: The Ultimate Challenge
    PRIME       // Year 16-20: The Final Boss
};

struct NemesisEncounter {
    int yearNumber;              // Which year (1-20)
    double gameTimeSeconds;      // When it appeared
    bool defeated;               // Did player win?
    int defeatsCount;           // How many times player lost to this nemesis
    std::string nemesisName;
    NemesisType type;
    int powerLevel;             // How many times this nemesis has evolved
};

class NemesisSystem {
public:
    NemesisSystem();
    
    // Check if nemesis should appear based on elapsed game time
    bool shouldNemesisAppear(double elapsedSeconds, double lastNemesisTime);

    void generateNemesisPatents(Monster &nemesis, int count, int playerSeries, NemesisType type);

    // Generate a nemesis boss based on game time and player level
    Monster generateNemesis(int playerSeries, double elapsedSeconds, const std::string& playerName);
    
    // Record when nemesis is defeated - this powers up the nemesis!
    void recordNemesisDefeat(int yearNumber, double gameTimeSeconds, NemesisType type, const std::string& nemesisName);
    
    // Track nemesis encounters
    void recordEncounter(const NemesisEncounter& encounter);
    std::vector<NemesisEncounter> getEncounterHistory() const { return encounterHistory_; }
    
    // Get time until next nemesis
    double getTimeUntilNextNemesis(double elapsedSeconds) const;
    
    // Get current nemesis type based on game time
    NemesisType getNemesisType(double elapsedSeconds) const;
    
    // Get current power level (how many times nemesis has been defeated)
    int getNemesisPowerLevel() const { return nemesisPowerLevel_; }
    
    // DEBUG: Force a nemesis encounter at any time point
    Monster debugGenerateNemesis(int playerSeries, double debugTimeSeconds, const std::string& playerName);
    
private:
    std::vector<NemesisEncounter> encounterHistory_;
    int nemesisPowerLevel_;  // Increases by 1 each time nemesis is defeated (1.1x multiplier per level)
    
    // Calculate nemesis stats using similar logic to requiredFundingForNextLevel
    struct NemesisStats {
        int health;
        int maxHealth;
        int attack;
        int defense;
        int speed;
        int moneyReward;
    };
    
    NemesisStats calculateNemesisStats(int playerSeries, double elapsedSeconds, NemesisType type, int powerLevel);
    std::string generateNemesisName(NemesisType type, int powerLevel, const std::string& playerName);
    std::string scramblePlayerName(const std::string& playerName);
};

#endif // NEMESISSYSTEM_H