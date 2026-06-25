#include <iostream>
#include <string>
#include <unordered_map>
#include <iomanip>
#include <filesystem>
#include <stdexcept>
#include "FileManager.h"
#include "Huffman.h"
#include "Compressor.h"

namespace fs = std::filesystem;

// ============================================================================
// CLI Display Helper Functions
// ============================================================================
void printHeader() {
    std::cout << "\n=======================================================\n";
    std::cout << "               HUFFMAN FILE COMPRESSOR                 \n";
    std::cout << "=======================================================\n";
}

void printMenu() {
    std::cout << "\n[1] Compress File\n";
    std::cout << "[2] Decompress File\n";
    std::cout << "[3] Show Compression Statistics (Last Session / Custom)\n";
    std::cout << "[4] Display Huffman Codes & Visual Tree\n";
    std::cout << "[5] Exit\n";
    std::cout << "\nEnter your choice (1-5): ";
}

std::string getInputWithDefault(const std::string& prompt, const std::string& defaultValue) {
    std::cout << prompt << " [" << defaultValue << "]: ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        return defaultValue;
    }
    return input;
}

// Utility to verify that two files are identical byte-for-byte
bool verifyFilesMatch(const std::string& path1, const std::string& path2) {
    if (!FileManager::fileExists(path1) || !FileManager::fileExists(path2)) {
        return false;
    }
    
    size_t size1 = FileManager::getFileSize(path1);
    size_t size2 = FileManager::getFileSize(path2);
    if (size1 != size2) {
        return false;
    }
    
    // Read and compare contents
    try {
        std::string content1 = FileManager::readTextFile(path1);
        std::string content2 = FileManager::readTextFile(path2);
        return content1 == content2;
    } catch (...) {
        return false;
    }
}

// ============================================================================
// Main Application Loop
// ============================================================================
int main() {
    // Ensure output directory exists
    try {
        fs::create_directories("output");
    } catch (...) {
        // Suppress if fails, will fail later on file open
    }

    bool running = true;
    Compressor::CompressionStats lastStats;
    bool hasLastStats = false;
    std::string lastOrigPath = "";
    std::string lastCompPath = "";

    while (running) {
        printHeader();
        printMenu();
        
        std::string choiceStr;
        std::getline(std::cin, choiceStr);
        if (choiceStr.empty()) continue;
        
        int choice = 0;
        try {
            choice = std::stoi(choiceStr);
        } catch (...) {
            std::cout << "\nInvalid choice. Please enter a number between 1 and 5.\n";
            continue;
        }

        switch (choice) {
            case 1: { // Compress File
                std::cout << "\n--- FILE COMPRESSION ---\n";
                std::string src = getInputWithDefault("Enter source text file path", "input/sample.txt");
                
                // Construct default destination path
                fs::path srcPath(src);
                std::string defaultDest = "output/" + srcPath.stem().string() + ".huf";
                std::string dest = getInputWithDefault("Enter destination compressed file path", defaultDest);
                
                std::cout << "\nProcessing compression. Please wait...\n";
                try {
                    lastStats = Compressor::compress(src, dest);
                    hasLastStats = true;
                    lastOrigPath = src;
                    lastCompPath = dest;
                    
                    // Display report in console
                    std::string report = Compressor::generateReport(lastStats, src, dest);
                    std::cout << "\n" << report << "\n";
                    
                    // Save report to disk
                    std::string reportPath = "output/" + srcPath.stem().string() + "_report.txt";
                    FileManager::writeTextFile(reportPath, report);
                    std::cout << "Compression report saved to: " << reportPath << "\n";
                    std::cout << "Compressed file successfully written to: " << dest << "\n";
                    
                } catch (const std::exception& e) {
                    std::cerr << "\n[ERROR] Compression failed: " << e.what() << "\n";
                }
                break;
            }
            
            case 2: { // Decompress File
                std::cout << "\n--- FILE DECOMPRESSION ---\n";
                std::string src = getInputWithDefault("Enter compressed binary file path (.huf)", "output/sample.huf");
                
                // Construct default destination path
                fs::path srcPath(src);
                std::string defaultDest = "output/" + srcPath.stem().string() + "_decomp.txt";
                std::string dest = getInputWithDefault("Enter destination decompressed file path", defaultDest);
                
                std::cout << "\nProcessing decompression. Please wait...\n";
                try {
                    double duration = Compressor::decompress(src, dest);
                    std::cout << "\nDecompression finished in " << std::fixed << std::setprecision(2) << duration << " ms.\n";
                    std::cout << "Decompressed file written to: " << dest << "\n";
                    
                    // Perform verification check
                    std::string originalFile = "";
                    if (hasLastStats && lastCompPath == src) {
                        originalFile = lastOrigPath;
                    } else {
                        // Guess original location in input/
                        std::string guessedPath = "input/" + srcPath.stem().string() + ".txt";
                        if (FileManager::fileExists(guessedPath)) {
                            originalFile = guessedPath;
                        }
                    }
                    
                    if (!originalFile.empty() && FileManager::fileExists(originalFile)) {
                        std::cout << "\nChecking integrity against original file: " << originalFile << "...\n";
                        if (verifyFilesMatch(originalFile, dest)) {
                            std::cout << "[SUCCESS] Integrity Verified! Decompressed output is a 100% exact match.\n";
                        } else {
                            std::cout << "[WARNING] Integrity Check Mismatch! The files do not match.\n";
                        }
                    } else {
                        std::cout << "\n[NOTE] Original file not found in cache for byte-match verification.\n";
                    }
                    
                } catch (const std::exception& e) {
                    std::cerr << "\n[ERROR] Decompression failed: " << e.what() << "\n";
                }
                break;
            }
            
            case 3: { // Show Compression Statistics
                std::cout << "\n--- COMPRESSION STATISTICS ---\n";
                if (hasLastStats) {
                    std::cout << "Displaying stats from last session operation:\n";
                    std::cout << Compressor::generateReport(lastStats, lastOrigPath, lastCompPath) << "\n";
                } else {
                    std::cout << "No compression run in this session yet. Let's compare files manually.\n";
                    std::string orig = getInputWithDefault("Enter original file path", "input/sample.txt");
                    std::string comp = getInputWithDefault("Enter compressed file path", "output/sample.huf");
                    
                    if (FileManager::fileExists(orig) && FileManager::fileExists(comp)) {
                        Compressor::CompressionStats customStats;
                        customStats.originalSize = FileManager::getFileSize(orig);
                        customStats.compressedSize = FileManager::getFileSize(comp);
                        if (customStats.originalSize > 0) {
                            customStats.compressionRatio = (static_cast<double>(customStats.compressedSize) / customStats.originalSize) * 100.0;
                            customStats.spaceSaved = (1.0 - (static_cast<double>(customStats.compressedSize) / customStats.originalSize)) * 100.0;
                        }
                        std::cout << "\n" << Compressor::generateReport(customStats, orig, comp) << "\n";
                    } else {
                        std::cout << "[ERROR] One or both files do not exist.\n";
                    }
                }
                break;
            }
            
            case 4: { // Display Huffman Codes & Visual Tree
                std::cout << "\n--- HUFFMAN CODES & TREE VISUALIZATION ---\n";
                std::string filepath = getInputWithDefault("Enter source text file path for tree visualization", "input/sample.txt");
                
                try {
                    std::string content = FileManager::readTextFile(filepath);
                    if (content.empty()) {
                        std::cout << "[INFO] File is empty. Cannot generate codes/tree for an empty file.\n";
                        break;
                    }
                    
                    std::unordered_map<char, size_t> frequencies;
                    for (char c : content) {
                        frequencies[c]++;
                    }
                    
                    HuffmanTree tree;
                    tree.buildFromFrequencies(frequencies);
                    std::unordered_map<char, std::string> codes = tree.generateCodes();
                    
                    // Display Encoding Table
                    std::cout << "\n=========================================\n";
                    std::cout << "              ENCODING TABLE             \n";
                    std::cout << "=========================================\n";
                    std::cout << std::left << std::setw(12) << "Character" 
                              << std::setw(12) << "Frequency" 
                              << "Huffman Code\n";
                    std::cout << "-----------------------------------------\n";
                    for (const auto& pair : codes) {
                        char c = pair.first;
                        std::string charRepr = "";
                        if (c == '\n') charRepr = "\\n (Newline)";
                        else if (c == '\t') charRepr = "\\t (Tab)";
                        else if (c == '\r') charRepr = "\\r (Carriage)";
                        else if (c == ' ') charRepr = "' ' (Space)";
                        else charRepr = std::string(1, c);
                        
                        std::cout << std::left << std::setw(12) << charRepr 
                                  << std::setw(12) << frequencies[c] 
                                  << pair.second << "\n";
                    }
                    std::cout << "=========================================\n";
                    
                    // Display Huffman Tree structure
                    std::cout << "\n=========================================\n";
                    std::cout << "         HUFFMAN TREE STRUCTURE          \n";
                    std::cout << "       (R: Right Child, L: Left Child)   \n";
                    std::cout << "=========================================\n";
                    tree.printTreeVisual();
                    std::cout << "=========================================\n";
                    
                } catch (const std::exception& e) {
                    std::cerr << "\n[ERROR] Failed to visualize tree: " << e.what() << "\n";
                }
                break;
            }
            
            case 5: { // Exit
                std::cout << "\nThank you for using Huffman File Compressor! Goodbye.\n";
                running = false;
                break;
            }
            
            default: {
                std::cout << "\nInvalid choice. Please enter a number between 1 and 5.\n";
                break;
            }
        }
        
        if (running) {
            std::cout << "\nPress Enter to return to the main menu...";
            std::string dummy;
            std::getline(std::cin, dummy);
        }
    }
    
    return 0;
}
