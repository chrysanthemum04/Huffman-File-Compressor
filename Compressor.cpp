#include "Compressor.h"
#include "FileManager.h"
#include "Huffman.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <stdexcept>

// ============================================================================
// Bit-Level Writing Helper
// ============================================================================
class BitWriter {
private:
    std::vector<uint8_t> buffer;
    uint8_t currentByte;
    int bitCount; // Tracker for current bit position (0-7, MSB to LSB)

public:
    BitWriter() : currentByte(0), bitCount(0) {}

    void writeBit(bool bit) {
        if (bit) {
            // Write bit to current position from MSB (bit 7) to LSB (bit 0)
            currentByte |= (1 << (7 - bitCount));
        }
        bitCount++;
        
        // If byte is complete, push it to buffer and reset
        if (bitCount == 8) {
            buffer.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }

    void writeBits(const std::string& bitString) {
        for (char c : bitString) {
            writeBit(c == '1');
        }
    }

    /**
     * @brief Flush remaining bits, padding with 0s if necessary.
     * @return The number of padding bits added (0-7).
     */
    uint8_t flush() {
        uint8_t paddingBits = 0;
        if (bitCount > 0) {
            paddingBits = 8 - bitCount;
            // Unwritten bits are naturally 0 because currentByte starts at 0
            buffer.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
        return paddingBits;
    }

    const std::vector<uint8_t>& getBytes() const {
        return buffer;
    }
};

// ============================================================================
// Bit-Level Reading Helper
// ============================================================================
class BitReader {
private:
    const std::vector<uint8_t>& buffer;
    size_t byteIndex;
    int bitIndex; // Tracker for current bit position (0-7, MSB to LSB)
    size_t totalBits;

public:
    BitReader(const std::vector<uint8_t>& buf, uint8_t paddingBits)
        : buffer(buf), byteIndex(0), bitIndex(0) {
        totalBits = buf.empty() ? 0 : (buf.size() * 8 - paddingBits);
    }

    bool hasNext() const {
        if (buffer.empty()) return false;
        size_t currentBitPos = byteIndex * 8 + bitIndex;
        return currentBitPos < totalBits;
    }

    bool readBit() {
        if (!hasNext()) {
            throw std::runtime_error("Attempted to read past the end of the bitstream.");
        }
        
        bool bit = (buffer[byteIndex] & (1 << (7 - bitIndex))) != 0;
        bitIndex++;
        
        if (bitIndex == 8) {
            bitIndex = 0;
            byteIndex++;
        }
        return bit;
    }
};

// ============================================================================
// Portable Serialization Helpers (Little-Endian)
// ============================================================================
static void writeUint32(std::vector<uint8_t>& dest, uint32_t value) {
    dest.push_back(value & 0xFF);
    dest.push_back((value >> 8) & 0xFF);
    dest.push_back((value >> 16) & 0xFF);
    dest.push_back((value >> 24) & 0xFF);
}

static uint32_t readUint32(const std::vector<uint8_t>& src, size_t& index) {
    if (index + 4 > src.size()) {
        throw std::runtime_error("Malformed file: Attempted to read past header boundary.");
    }
    uint32_t value = src[index] |
                    (src[index + 1] << 8) |
                    (src[index + 2] << 16) |
                    (src[index + 3] << 24);
    index += 4;
    return value;
}

// ============================================================================
// Compressor Implementation
// ============================================================================

Compressor::CompressionStats Compressor::compress(const std::string& srcPath, const std::string& destPath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    CompressionStats stats;
    std::string content = FileManager::readTextFile(srcPath);
    stats.originalSize = content.size();

    // Handle empty file edge case
    if (content.empty()) {
        // Create an empty file at target path
        FileManager::writeBinaryFile(destPath, {});
        
        auto endTime = std::chrono::high_resolution_clock::now();
        stats.durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        stats.compressedSize = 0;
        stats.compressionRatio = 0.0;
        stats.spaceSaved = 0.0;
        return stats;
    }

    // 1. Calculate frequencies
    std::unordered_map<char, size_t> frequencies;
    for (char c : content) {
        frequencies[c]++;
    }

    // 2. Build Huffman Tree
    HuffmanTree tree;
    tree.buildFromFrequencies(frequencies);

    // 3. Generate codes
    std::unordered_map<char, std::string> codes = tree.generateCodes();

    // 4. Construct Output Buffer with Header
    std::vector<uint8_t> outputBuffer;

    // Header Part A: Number of unique characters (4 bytes)
    uint32_t uniqueCharsCount = static_cast<uint32_t>(frequencies.size());
    writeUint32(outputBuffer, uniqueCharsCount);

    // Header Part B: Frequency map details
    for (const auto& pair : frequencies) {
        outputBuffer.push_back(static_cast<uint8_t>(pair.first)); // 1 byte character
        writeUint32(outputBuffer, static_cast<uint32_t>(pair.second)); // 4 bytes frequency
    }

    // Header Part C: Padding bits count (1 byte placeholder, we'll write this value after flushing)
    size_t paddingIndex = outputBuffer.size();
    outputBuffer.push_back(0);

    // 5. Compress bitstream
    BitWriter bitWriter;
    for (char c : content) {
        bitWriter.writeBits(codes[c]);
    }
    
    // Flush bit writer and retrieve actual padding bits
    uint8_t paddingBits = bitWriter.flush();
    outputBuffer[paddingIndex] = paddingBits;

    // Append compressed bitstream to output buffer
    const auto& bitstreamBytes = bitWriter.getBytes();
    outputBuffer.insert(outputBuffer.end(), bitstreamBytes.begin(), bitstreamBytes.end());

    // 6. Write output binary file
    FileManager::writeBinaryFile(destPath, outputBuffer);
    stats.compressedSize = outputBuffer.size();

    auto endTime = std::chrono::high_resolution_clock::now();
    stats.durationMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Compute stats metrics
    if (stats.originalSize > 0) {
        stats.compressionRatio = (static_cast<double>(stats.compressedSize) / stats.originalSize) * 100.0;
        stats.spaceSaved = (1.0 - (static_cast<double>(stats.compressedSize) / stats.originalSize)) * 100.0;
    }

    return stats;
}

double Compressor::decompress(const std::string& srcPath, const std::string& destPath) {
    auto startTime = std::chrono::high_resolution_clock::now();

    std::vector<uint8_t> compressedBytes = FileManager::readBinaryFile(srcPath);

    // Handle empty file edge case
    if (compressedBytes.empty()) {
        FileManager::writeTextFile(destPath, "");
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(endTime - startTime).count();
    }

    size_t byteIndex = 0;

    // 1. Read header part A: unique characters count
    uint32_t uniqueCharsCount = readUint32(compressedBytes, byteIndex);
    if (uniqueCharsCount == 0) {
        FileManager::writeTextFile(destPath, "");
        auto endTime = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(endTime - startTime).count();
    }

    // 2. Read header part B: frequency table
    std::unordered_map<char, size_t> frequencies;
    for (uint32_t i = 0; i < uniqueCharsCount; ++i) {
        if (byteIndex >= compressedBytes.size()) {
            throw std::runtime_error("Malformed file: Frequency header is truncated.");
        }
        char character = static_cast<char>(compressedBytes[byteIndex]);
        byteIndex++;
        
        uint32_t freq = readUint32(compressedBytes, byteIndex);
        frequencies[character] = freq;
    }

    // 3. Read header part C: padding bits count
    if (byteIndex >= compressedBytes.size()) {
        throw std::runtime_error("Malformed file: Padding bits byte is missing.");
    }
    uint8_t paddingBits = compressedBytes[byteIndex];
    byteIndex++;

    // 4. Reconstruct Huffman Tree
    HuffmanTree tree;
    tree.buildFromFrequencies(frequencies);
    const HuffmanNode* root = tree.getRoot();
    if (root == nullptr) {
        throw std::runtime_error("Malformed file: Huffman tree reconstruction failed.");
    }

    // Extract remaining bitstream bytes
    std::vector<uint8_t> bitstreamBytes(compressedBytes.begin() + byteIndex, compressedBytes.end());
    BitReader bitReader(bitstreamBytes, paddingBits);

    // 5. Decode bitstream using Huffman Tree
    std::string decompressedText;
    size_t totalCharacters = 0;
    for (const auto& pair : frequencies) {
        totalCharacters += pair.second;
    }

    const HuffmanNode* currentNode = root;
    size_t decodedCharsCount = 0;

    while (bitReader.hasNext() && decodedCharsCount < totalCharacters) {
        bool bit = bitReader.readBit();
        
        currentNode = bit ? currentNode->right : currentNode->left;
        if (currentNode == nullptr) {
            throw std::runtime_error("Malformed file: Invalid bitstream navigation path.");
        }

        if (currentNode->isLeaf()) {
            decompressedText.push_back(currentNode->data);
            decodedCharsCount++;
            currentNode = root; // Go back to root for the next character
        }
    }

    // Verify integrity
    if (decodedCharsCount != totalCharacters) {
        throw std::runtime_error("Decryption failed: Characters count mismatch. File may be corrupted.");
    }

    // 6. Write decompressed text to file
    FileManager::writeTextFile(destPath, decompressedText);

    auto endTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

std::string Compressor::generateReport(const CompressionStats& stats,
                                       const std::string& srcPath,
                                       const std::string& destPath) {
    std::stringstream ss;
    ss << "======================================================================\n";
    ss << "                     HUFFMAN COMPRESSION REPORT                       \n";
    ss << "======================================================================\n";
    ss << "Original File Path:    " << srcPath << "\n";
    ss << "Compressed File Path:  " << destPath << "\n";
    ss << "----------------------------------------------------------------------\n";
    ss << "Original File Size:    " << stats.originalSize << " bytes\n";
    ss << "Compressed File Size:  " << stats.compressedSize << " bytes\n";
    
    // Display compression ratio
    ss << std::fixed << std::setprecision(2);
    ss << "Compression Ratio:     " << stats.compressionRatio << "%\n";
    ss << "Space Saved:           " << stats.spaceSaved << "%\n";
    
    // Display ratio formatting (e.g. 2.45:1)
    if (stats.compressedSize > 0) {
        double ratioFactor = static_cast<double>(stats.originalSize) / stats.compressedSize;
        ss << "Reduction Factor:      " << ratioFactor << ":1\n";
    } else {
        ss << "Reduction Factor:      N/A\n";
    }
    
    ss << "Processing Time:       " << stats.durationMs << " ms\n";
    ss << "======================================================================\n";
    return ss.str();
}
