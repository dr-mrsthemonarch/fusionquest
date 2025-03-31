#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

enum class LogLevel {
    ERROR,
    INFO,
    DEBUG
};

class Logger {
public:
    // Constructor: Opens the log file
    Logger(const std::string& filename = "log.txt") : log_file_(filename, std::ios::out | std::ios::app) {
        if (!log_file_.is_open()) {
            std::cerr << "Failed to open log file: " << filename << std::endl;
        }
    }

    // Destructor: Closes the log file
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    // Check if the log file is open
    bool IsLogFileOpen() const {
        return log_file_.is_open();
    }

    // Set the log level
    void SetLogLevel(LogLevel level) {
        log_level_ = level;
    }

    // Log a message with timestamp, thread ID, and log level
    void Log(const std::string& message, LogLevel level = LogLevel::INFO) {
        if (level > log_level_) return; // Skip logging if the message level is lower than the configured level

        std::lock_guard<std::mutex> lock(mutex_); // Ensure thread safety

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_tm = *std::localtime(&now_time_t);

        // Format timestamp
        std::ostringstream timestamp_stream;
        timestamp_stream << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
        std::string timestamp = timestamp_stream.str();

        // Get thread ID
        std::thread::id thread_id = std::this_thread::get_id();
        std::ostringstream thread_id_stream;
        thread_id_stream << thread_id;
        std::string thread_id_str = thread_id_stream.str();

        // Write log entry
        if (log_file_.is_open()) {
            log_file_ << "[" << timestamp << "] [Thread " << thread_id_str << "] [" << LogLevelToString(level) << "] " << message << std::endl;
        }
    }

private:
    // Convert LogLevel to string
    std::string LogLevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::INFO: return "INFO";
            case LogLevel::DEBUG: return "DEBUG";
            default: return "UNKNOWN";
        }
    }

    std::ofstream log_file_; // Log file stream
    std::mutex mutex_;       // Mutex for thread safety
    LogLevel log_level_ = LogLevel::INFO; // Default log level
};

#endif // LOGGER_H