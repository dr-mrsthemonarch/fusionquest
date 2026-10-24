#ifndef TUNING_H
#define TUNING_H

#include <string>
#include <unordered_map>
#include <vector>

// central registry of every balance factor. game code reads T("key"); the debug tuner edits live.
// defaults live in Tuning.cpp. config/tuning.txt (embedded) overrides them. with --debug-tune a
// ./tuning.txt on disk overrides again and the tuner can write it. copy it into config/ to ship.
struct TuneParam {
    std::string key, group, desc;
    double value = 0, def = 0, step = 1;
    bool modified() const { return value != def; }
};

class Tuning {
public:
    static Tuning& I();

    void init(bool debugTune);                  // defaults -> embedded overrides -> (debug) disk overrides
    bool enabled() const { return enabled_; }   // --debug-tune

    double get(const char* key) const;          // unknown key -> 0 (logged once)
    bool set(const std::string& key, double v);
    void reset(size_t index);
    void resetAll();
    const std::vector<TuneParam>& params() const { return params_; }
    bool save(const std::string& path) const;

private:
    Tuning();
    void loadOverrides(std::istream& in);
    std::vector<TuneParam> params_;
    std::unordered_map<std::string, size_t> index_;
    bool enabled_ = false;
    mutable bool warned_ = false;
};

inline double T(const char* key) { return Tuning::I().get(key); }

#endif
