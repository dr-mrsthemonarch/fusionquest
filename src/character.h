// character.h
#pragma once
#include <string>

class Character {
public:
    // Basic info
    std::string name = "John Bekx";
    int race_index = 4;
    int class_index = 0;
    int fusion_index = 4;

    // Base stats
    int hustle = 1;
    int agility = 1;
    float busfactor = 1;
    int techdebt = 1;
    int ndas = 0;
    int luck = 100;
    int Synergy = 0;

    //Game Engine derived stats
    int Series = 1;

    // Derived stats (could be calculated from base stats)
    float health = 0.8f;
    float mana = 0.6f;
    float stamina = 0.7f;
    float attack = 0.5f;
    float defense = 0.9f;
    float magic = 0.4f;

    // Progress stats
    float experience = 0.45f;
    float progress = 0.65f;

    // Helper methods
    float total_stats() const {
        return (hustle + agility - busfactor + techdebt + ndas + luck + Synergy);
    }
};