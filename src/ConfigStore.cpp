#include "../include/ConfigStore.h"
#include "../include/ConfigCipher.h"
#include <fstream>

namespace {
std::string normalize(std::string p) {
    for (char& c : p) if (c == '\\') c = '/';
    size_t pos = p.rfind("config/");
    if (pos != std::string::npos) p = p.substr(pos + 7);
    return p;
}
}  // namespace

ConfigFile::ConfigFile(const std::string& path) {
#ifdef FQ_EXTERNAL_CONFIG
    {
        std::ifstream f(path, std::ios::binary);
        if (f) {
            std::stringstream ss;
            ss << f.rdbuf();
            str(ss.str());
            open_ = true;
            return;
        }
    }
#endif
    const std::string key = normalize(path);
    for (size_t i = 0; i < kEmbeddedFileCount; ++i) {
        if (key != kEmbeddedFiles[i].name) continue;
        std::string s(kEmbeddedFiles[i].size, '\0');
        for (size_t j = 0; j < s.size(); ++j)
            s[j] = static_cast<char>(kEmbeddedFiles[i].data[j] ^ configKey(j, i));
        str(s);
        open_ = true;
        return;
    }
    setstate(std::ios::failbit);
}

bool validateConfig(std::string& error) {
    static const char* required[] = {
        "equipment/adjectives.txt", "materials.txt",
        "mobs/bosses.txt", "mobs/mobs.txt", "mobs/titles.txt", "mobs/adjectives.txt", "mobs/mythical_nouns.txt",
        "items/adjectives.txt", "items/office_nouns.txt", "items/science_nouns.txt", "items/fancy_suffix.txt",
        "items/mythical_nouns.txt", "items/it_words.txt", "items/company_suffixes.txt", "items/value_modifiers.txt",
        "items/patent_names.txt", "items/named_patents.txt",
        "quests/main_quests.txt", "quests/balance.txt", "quests/preamble.txt",
        "quests/hustle_names.txt", "quests/dev_names.txt", "quests/sprint_names.txt"};
#ifndef FQ_EXTERNAL_CONFIG
    if (kEmbeddedFileCount == 0) { error = "embedded config is empty (build step failed?)"; return false; }
#endif
    for (const char* name : required) {
        ConfigFile f(std::string("config/") + name);
        std::string first;
        bool any = false;
        if (f.is_open()) {
            std::string line;
            while (std::getline(f, line)) {
                if (line.find_first_not_of(" \t\r\n") != std::string::npos) { any = true; break; }
            }
        }
        if (!any) { error = std::string("missing or empty config file: ") + name; return false; }
    }
    return true;
}
