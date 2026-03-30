#include "Reader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::string readFile(const std::string &filePath)
{
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        throw std::runtime_error("Failed to open " + filePath);
    }

    std::ostringstream buffer;
    buffer << inputFile.rdbuf();
    return buffer.str();
}