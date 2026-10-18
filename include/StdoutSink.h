#ifndef STDOUTSINK_H
#define STDOUTSINK_H

#include <fstream>
#include <iostream>

// ftxui draws the screen through std::cout, so cout can NOT be redirected globally.
// engine/Character/Patent code prints a lot of debug text though. this guard sends
// cout/cerr to fusionquest_stdout.log only while alive (engine tick, save/load, init).
// all of that runs on the ui thread, outside ftxui's draw call, so no races.
class StdoutSink {
public:
    StdoutSink()
        : old_cout_(std::cout.rdbuf(file().rdbuf())), old_cerr_(std::cerr.rdbuf(file().rdbuf())) {}
    ~StdoutSink() {
        std::cout.rdbuf(old_cout_);
        std::cerr.rdbuf(old_cerr_);
    }
    StdoutSink(const StdoutSink&) = delete;
    StdoutSink& operator=(const StdoutSink&) = delete;

private:
    static std::ofstream& file() {
        static std::ofstream f("fusionquest_stdout.log", std::ios::app);
        return f;
    }
    std::streambuf* old_cout_;
    std::streambuf* old_cerr_;
};

#endif
