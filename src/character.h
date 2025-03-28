// character.h
#pragma once
#include <string>

class Character {
public:
    // Basic info
    std::string name = "Burt von Skinny";
    int race_index = 0;
    int class_index = 0;
    int fusion_index = 0;

    // Base stats
    int hustle = 1;
    int agility = 1;
    float busfactor = 1;
    int techdebt = 1;
    int ndas = 0;
    int luck = 100;
    int patents = 0;

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
    int total_stats() const {
        return hustle + agility + busfactor + techdebt;
    }

    void update_derived_stats() {
        // Example of updating derived stats based on base stats
        health = std::min(1.0f, techdebt * 0.04f);
        attack = std::min(1.0f, hustle * 0.04f);
        defense = std::min(1.0f, (hustle + techdebt) * 0.02f);
        mana = std::min(1.0f, busfactor * 0.04f);
        magic = std::min(1.0f, busfactor * 0.04f);
        stamina = std::min(1.0f, agility * 0.04f);
    }
};