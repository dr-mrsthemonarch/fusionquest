#ifndef CONFIGSTORE_H
#define CONFIGSTORE_H

#include <cstddef>
#include <sstream>
#include <string>

// config files are compiled into the executable (generated ConfigData.cpp, see tools/embed_config.cpp).
struct EmbeddedFile {
    const char* name;            // path relative to config/, e.g. "items/adjectives.txt"
    const unsigned char* data;   // obfuscated bytes
    size_t size;
};
extern const EmbeddedFile kEmbeddedFiles[];
extern const size_t kEmbeddedFileCount;

// drop-in for std::ifstream on config paths ("config/items/x.txt" or "items/x.txt").
// reads from embedded data. with -DFQ_EXTERNAL_CONFIG (dev builds) disk is tried first.
class ConfigFile : public std::istringstream {
public:
    explicit ConfigFile(const std::string& path);
    bool is_open() const { return open_; }
    void close() {}
private:
    bool open_ = false;
};

// startup sanity check: every required config file present and non-empty.
// false + human readable error if the embedded data is missing/broken.
bool validateConfig(std::string& error);

#endif
