#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

/**
 * @class Compressor
 * @brief Coordinates the bit-level Huffman compression and decompression processes,
 *        computes statistics, and generates reports.
 */
class Compressor {
public:
    struct CompressionStats {
        size_t originalSize = 0;
        size_t compressedSize = 0;
        double compressionRatio = 0.0; // (compressedSize / originalSize) * 100%
        double spaceSaved = 0.0;       // (1 - compressedSize / originalSize) * 100%
        double durationMs = 0.0;       // Execution time in milliseconds
    };

    // Prevent instantiation of utility class
    Compressor() = delete;

    /**
     * @brief Compresses a text file to a binary file using Huffman Coding.
     * @param srcPath Path to the input text file.
     * @param destPath Path to the output compressed file (.huf).
     * @return CompressionStats struct containing sizes and compression ratios.
     * @throws std::runtime_error if file reading/writing fails or compression is invalid.
     */
    static CompressionStats compress(const std::string& srcPath, const std::string& destPath);

    /**
     * @brief Decompresses a binary file (.huf) back to a text file.
     * @param srcPath Path to the compressed binary file.
     * @param destPath Path to the output decompressed text file.
     * @return Double value representing decompression execution time in milliseconds.
     * @throws std::runtime_error if file is malformed or writing fails.
     */
    static double decompress(const std::string& srcPath, const std::string& destPath);

    /**
     * @brief Generates a formatted text report of the compression results.
     * @param stats CompressionStats from the compression process.
     * @param srcPath File path of the original file.
     * @param destPath File path of the compressed file.
     * @return Formatted string report.
     */
    static std::string generateReport(const CompressionStats& stats,
                                      const std::string& srcPath,
                                      const std::string& destPath);
};
