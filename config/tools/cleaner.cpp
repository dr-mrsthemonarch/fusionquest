#include <fstream>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <filesystem>
#include <cctype>
#include <locale>

//a tool to clean up and sort nonsense from AI generated lists of words.
// Function to trim whitespace from a string
static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    if (start > end) return ""; // Return empty string if the line is empty
    return str.substr(start, end - start + 1);
}

int main(int argc, char* argv[]) {
    // Check if any arguments were provided
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path>" << std::endl;
        std::cerr << "Either a directory or a file can be provided as argument." << std::endl;
        return 1;
    }

    std::string path = argv[1];
    namespace fs = std::filesystem;

    // Check if the provided path is a file or directory
    if (fs::is_regular_file(path)) {
        // Process single file
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << path << std::endl;
            return 1;
        }

        std::set<std::string> uniqueWords;
        std::string word;

        while (std::getline(file, word)) {
            // Trim leading/trailing whitespace
            auto trimmed = trim(word);
            if (trimmed.empty()) continue; // Skip empty lines

            // Insert the trimmed word (this is safer than modifying directly)
            uniqueWords.insert(trimmed);
        }

        // Convert set to vector for sorting
        std::vector<std::string> sortedWords(uniqueWords.begin(), uniqueWords.end());
        std::sort(sortedWords.begin(), sortedWords.end());

        // Write the sorted list to a file with the same name
        std::ofstream outputFile(path);
        if (!outputFile.is_open()) {
            std::cerr << "Error creating output file: " << path << std::endl;
            return 1;
        }

        for (const auto& w : sortedWords) {
            outputFile << w << std::endl;
        }

        std::cout << "Processed file: " << path << std::endl;
        return 0;
    }
    else if (fs::is_directory(path)) {
        // Process directory recursively (same as before)
        std::cout << "Processing directory: " << path << std::endl;

        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::ifstream file(entry.path());
                if (!file.is_open()) {
                    std::cerr << "Error opening file: " << entry.path() << std::endl;
                    continue;
                }

                std::set<std::string> uniqueWords;
                std::string word;

                while (std::getline(file, word)) {
                    // Trim leading/trailing whitespace
                    auto trimmed = trim(word);
                    if (trimmed.empty()) continue; // Skip empty lines

                    // Insert the trimmed word (this is safer than modifying directly)
                    uniqueWords.insert(trimmed);
                }

                // Convert set to vector for sorting
                std::vector<std::string> sortedWords(uniqueWords.begin(), uniqueWords.end());
                std::sort(sortedWords.begin(), sortedWords.end());

                // Write the sorted list to a file with the same name
                std::ofstream outputFile(entry.path());
                if (!outputFile.is_open()) {
                    std::cerr << "Error creating output file: " << entry.path() << std::endl;
                    continue;
                }

                for (const auto& w : sortedWords) {
                    outputFile << w << std::endl;
                }

                std::cout << "Processed: " << entry.path() << std::endl;
            }
        }

        return 0;
    }
    else {
        std::cerr << "Error: " << path << " is neither a file nor a directory." << std::endl;
        return 1;
    }
}