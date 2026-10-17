//
// Created by drmrsthemonarch on 23.11.25.
//

#ifndef PATENTDEF_H
#define PATENTDEF_H

#include <string>
#include "Equipment.h"
#include "Patent.h"
enum class PatentType;
enum class PatentTarget;
struct PatentDefinition {
    std::string name;
    PatentType type;
    int basePower;
    int cost;
    PatentTarget target;
    int seriesRequired;

    // Custom stat modifiers (optional)
    int hustleModifier = 0;
    int agilityModifier = 0;
    int ndasModifier = 0;
    int luckModifier = 0;
    int synergyModifier = 0;
    int technicalDebtModifier = 0;
    int duration = 0; // 0 = use default
};

#endif
