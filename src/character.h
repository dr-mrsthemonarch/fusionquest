// character.h
#pragma once
#include <string>

class Character {
public:
    // Basic info
    std::string name = "";
    int race_index = 0;
    int class_index = 0;

    // Base stats
    int strength = 1;
    int dexterity = 1;
    int intelligence = 1;
    int constitution = 1;

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
        return strength + dexterity + intelligence + constitution;
    }

    void update_derived_stats() {
        // Example of updating derived stats based on base stats
        health = std::min(1.0f, constitution * 0.04f);
        attack = std::min(1.0f, strength * 0.04f);
        defense = std::min(1.0f, (strength + constitution) * 0.02f);
        mana = std::min(1.0f, intelligence * 0.04f);
        magic = std::min(1.0f, intelligence * 0.04f);
        stamina = std::min(1.0f, dexterity * 0.04f);
    }
};