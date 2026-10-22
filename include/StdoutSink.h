#ifndef STDOUTSINK_H
#define STDOUTSINK_H

#include <fstream>
#include <functional>
#include <iostream>
#include <string>

// ftxui draws the screen through std::cout, so cout can NOT be redirected globally.
// engine/Character/Patent code prints a lot of detail text though (patent effects, level-ups,
// sales...). this guard captures cout/cerr only while alive (engine tick, save/load, init):
// every line goes to fusionquest_stdout.log AND to the line handler (engine's detail log).
// all of that runs on the ui thread, outside ftxui's draw call, so no races.
class StdoutSink {
public:
    using LineHandler = std::function<void(const std::string&)>;
    static void setLineHandler(LineHandler h) { handler() = std::move(h); }

    StdoutSink()
        : old_cout_(std::cout.rdbuf(&buf())), old_cerr_(std::cerr.rdbuf(&buf())) {}
    ~StdoutSink() {
        std::cout.rdbuf(old_cout_);
        std::cerr.rdbuf(old_cerr_);
    }
    StdoutSink(const StdoutSink&) = delete;
    StdoutSink& operator=(const StdoutSink&) = delete;

private:
    struct TeeBuf : std::streambuf {
        std::string line;
        std::ofstream file{"fusionquest_stdout.log", std::ios::app};

        int_type overflow(int_type c) override {
            if (c == traits_type::eof()) return traits_type::not_eof(c);
            char ch = static_cast<char>(c);
            file.put(ch);
            if (ch == '\n') flushLine();
            else line.push_back(ch);
            return c;
        }
        std::streamsize xsputn(const char* s, std::streamsize n) override {
            for (std::streamsize i = 0; i < n; ++i) overflow(traits_type::to_int_type(s[i]));
            return n;
        }
        int sync() override { file.flush(); return 0; }

        void flushLine() {
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
            // skip blanks and pure separator lines (=====, -----)
            bool meaningful = false;
            for (char ch : line)
                if (ch != ' ' && ch != '=' && ch != '-') { meaningful = true; break; }
            if (meaningful && handler()) handler()(line);
            line.clear();
        }
    };

    static TeeBuf& buf() { static TeeBuf b; return b; }
    static LineHandler& handler() { static LineHandler h; return h; }

    std::streambuf* old_cout_;
    std::streambuf* old_cerr_;
};

#endif
