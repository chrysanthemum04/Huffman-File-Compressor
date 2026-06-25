#pragma once

#include <unordered_map>
#include <string>
#include <queue>
#include <memory>

/**
 * @struct HuffmanNode
 * @brief Represents a node in the Huffman Tree.
 *        Uses recursive destructor to clean up child nodes automatically.
 */
struct HuffmanNode {
    char data;
    size_t frequency;
    HuffmanNode* left;
    HuffmanNode* right;

    // Constructor for leaf node
    HuffmanNode(char character, size_t freq)
        : data(character), frequency(freq), left(nullptr), right(nullptr) {}

    // Constructor for internal node
    HuffmanNode(size_t freq, HuffmanNode* leftChild, HuffmanNode* rightChild)
        : data('\0'), frequency(freq), left(leftChild), right(rightChild) {}

    // Destructor (recursively deletes children)
    ~HuffmanNode() {
        delete left;
        delete right;
    }

    // Helper to check if node is leaf
    bool isLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

/**
 * @struct CompareNode
 * @brief Comparator functor for the Min-Heap priority queue.
 *        Prioritizes nodes with lower frequency.
 */
struct CompareNode {
    bool operator()(const HuffmanNode* lhs, const HuffmanNode* rhs) const {
        return lhs->frequency > rhs->frequency;
    }
};

/**
 * @class HuffmanTree
 * @brief Manages Huffman tree construction, code generation, and visualization.
 */
class HuffmanTree {
private:
    HuffmanNode* root;

    // Helper for recursive code generation
    void generateCodesHelper(const HuffmanNode* node, const std::string& currentCode,
                             std::unordered_map<char, std::string>& codes) const;

    // Helper for printing tree structure visually
    void printTreeHelper(const HuffmanNode* node, int indent, const std::string& prefix) const;

public:
    HuffmanTree();
    ~HuffmanTree();

    // Delete copy operations to prevent double-deletion of raw pointers
    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree& operator=(const HuffmanTree&) = delete;

    // Implement move semantics
    HuffmanTree(HuffmanTree&& other) noexcept;
    HuffmanTree& operator=(HuffmanTree&& other) noexcept;

    /**
     * @brief Builds the Huffman Tree from a frequency map.
     * @param frequencies Unordered map of characters and their frequency counts.
     */
    void buildFromFrequencies(const std::unordered_map<char, size_t>& frequencies);

    /**
     * @brief Generates standard prefix-free binary codes for each character.
     * @return Map of character to binary string representation.
     */
    std::unordered_map<char, std::string> generateCodes() const;

    /**
     * @brief Visualizes the Huffman Tree in console.
     */
    void printTreeVisual() const;

    /**
     * @brief Returns the root of the tree (for decoding traversal).
     */
    const HuffmanNode* getRoot() const;

    /**
     * @brief Deallocates the tree and resets the root.
     */
    void clear();
};
