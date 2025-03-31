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
    float runway = 0.8f;
    float hype_energy = 0.8f;

    // Progress stats
    float experience = 0.45f;
    float progress = 0.65f;

    // Helper methods
    float total_stats() const {
        return (hustle + agility - busfactor + techdebt + ndas + luck + Synergy);
    }
};