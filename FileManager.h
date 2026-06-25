#pragma once

#include <string>
#include <vector>
#include <cstdint>

/**
 * @class FileManager
 * @brief Handles file operations for reading/writing text and binary files,
 *        as well as file size calculations and checks.
 */
class FileManager {
public:
    // Prevent instantiation of utility class
    FileManager() = delete;

    /**
     * @brief Reads an entire text file and returns its content.
     * @param filepath Path to the text file.
     * @return Content of the file as a std::string.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static std::string readTextFile(const std::string& filepath);

    /**
     * @brief Writes a string to a text file. Overwrites existing contents.
     * @param filepath Path to the text file.
     * @param content String content to write.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static void writeTextFile(const std::string& filepath, const std::string& content);

    /**
     * @brief Reads a binary file and returns its bytes.
     * @param filepath Path to the binary file.
     * @return Vector of raw bytes (uint8_t).
     * @throws std::runtime_error if the file cannot be opened.
     */
    static std::vector<uint8_t> readBinaryFile(const std::string& filepath);

    /**
     * @brief Writes raw bytes to a binary file. Overwrites existing contents.
     * @param filepath Path to the binary file.
     * @param data Vector of raw bytes.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static void writeBinaryFile(const std::string& filepath, const std::vector<uint8_t>& data);

    /**
     * @brief Returns the size of a file in bytes.
     * @param filepath Path to the file.
     * @return Size of the file in bytes, or 0 if it doesn't exist/is empty.
     */
    static size_t getFileSize(const std::string& filepath);

    /**
     * @brief Checks if a file exists and is accessible.
     * @param filepath Path to the file.
     * @return true if the file exists, false otherwise.
     */
    static bool fileExists(const std::string& filepath);
};
