//
// Created by drmrsthemonarch on 23.11.25.
//

#ifndef PATENTLIBRARY_H
#define PATENTLIBRARY_H

#include <istream>
#include <vector>
#include <string>
#include "PatentDefinition.h"

enum class PatentType;
enum class PatentTarget;
struct PatentDefinition;
class PatentLibrary {
public:
    PatentLibrary();

    bool loadFromFile(const std::string& filename);

    const PatentDefinition* getRandomNamedPatent() const;
    const PatentDefinition* getPatentByIndex(size_t index) const;
    size_t getPatentCount() const { return patents_.size(); }

private:
    std::vector<PatentDefinition> patents_;

    bool parsePatentBlock(std::istream& file, PatentDefinition& patent);
    std::string trim(const std::string& str);

    PatentType stringToPatentType(const std::string& str);
    PatentTarget stringToPatentTarget(const std::string& str);
};

#endif
