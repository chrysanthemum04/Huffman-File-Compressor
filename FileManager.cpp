#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

std::string FileManager::readTextFile(const std::string& filepath) {
    if (!fileExists(filepath)) {
        throw std::runtime_error("File not found: " + filepath);
    }
    
    std::ifstream file(filepath, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    
    auto size = fs::file_size(filepath);
    if (size == 0) {
        return "";
    }
    
    std::string content(size, '\0');
    file.read(&content[0], size);
    return content;
}

void FileManager::writeTextFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath, std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
    
    if (!content.empty()) {
        file.write(content.data(), content.size());
    }
}

std::vector<uint8_t> FileManager::readBinaryFile(const std::string& filepath) {
    if (!fileExists(filepath)) {
        throw std::runtime_error("File not found: " + filepath);
    }
    
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    
    auto size = fs::file_size(filepath);
    if (size == 0) {
        return {};
    }
    
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

void FileManager::writeBinaryFile(const std::string& filepath, const std::vector<uint8_t>& data) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filepath);
    }
    
    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }
}

size_t FileManager::getFileSize(const std::string& filepath) {
    try {
        if (fileExists(filepath)) {
            return fs::file_size(filepath);
        }
    } catch (...) {
        // Suppress and return 0
    }
    return 0;
}

bool FileManager::fileExists(const std::string& filepath) {
    try {
        return fs::exists(filepath) && fs::is_regular_file(filepath);
    } catch (...) {
        return false;
    }
}
