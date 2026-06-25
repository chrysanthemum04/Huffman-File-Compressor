#include "Huffman.h"
#include <iostream>

HuffmanTree::HuffmanTree() : root(nullptr) {}

HuffmanTree::~HuffmanTree() {
    clear();
}

HuffmanTree::HuffmanTree(HuffmanTree&& other) noexcept : root(other.root) {
    other.root = nullptr;
}

HuffmanTree& HuffmanTree::operator=(HuffmanTree&& other) noexcept {
    if (this != &other) {
        clear();
        root = other.root;
        other.root = nullptr;
    }
    return *this;
}

void HuffmanTree::clear() {
    delete root;
    root = nullptr;
}

const HuffmanNode* HuffmanTree::getRoot() const {
    return root;
}

void HuffmanTree::buildFromFrequencies(const std::unordered_map<char, size_t>& frequencies) {
    clear();
    if (frequencies.empty()) {
        return;
    }

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, CompareNode> minHeap;

    // 1. Create a leaf node for each character and add it to the min-heap
    for (const auto& pair : frequencies) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }

    // Edge case: Only one unique character in the file (e.g., "aaaaa" or single character file)
    if (minHeap.size() == 1) {
        HuffmanNode* leaf = minHeap.top();
        minHeap.pop();
        // Create an internal parent node so that we still have a path (e.g., left branch = "0")
        root = new HuffmanNode(leaf->frequency, leaf, nullptr);
        return;
    }

    // 2. Iterate until the size of heap reduces to 1
    while (minHeap.size() > 1) {
        // Pop the two nodes of lowest frequency
        HuffmanNode* leftChild = minHeap.top();
        minHeap.pop();

        HuffmanNode* rightChild = minHeap.top();
        minHeap.pop();

        // Create a new internal node with a frequency equal to the sum of the two nodes' frequencies.
        // The two nodes become children of this internal node.
        HuffmanNode* parent = new HuffmanNode(
            leftChild->frequency + rightChild->frequency,
            leftChild,
            rightChild
        );

        // Push the new node back into the min-heap
        minHeap.push(parent);
    }

    // The remaining node is the root node, and the tree is complete
    root = minHeap.top();
}

std::unordered_map<char, std::string> HuffmanTree::generateCodes() const {
    std::unordered_map<char, std::string> codes;
    if (root != nullptr) {
        generateCodesHelper(root, "", codes);
    }
    return codes;
}

void HuffmanTree::generateCodesHelper(const HuffmanNode* node, const std::string& currentCode,
                                     std::unordered_map<char, std::string>& codes) const {
    if (node == nullptr) {
        return;
    }

    // If it's a leaf node, store its code
    if (node->isLeaf()) {
        codes[node->data] = currentCode.empty() ? "0" : currentCode;
        return;
    }

    // Traverse Left (append '0') and Right (append '1')
    generateCodesHelper(node->left, currentCode + "0", codes);
    generateCodesHelper(node->right, currentCode + "1", codes);
}

void HuffmanTree::printTreeVisual() const {
    if (root == nullptr) {
        std::cout << "[Empty Tree]" << std::endl;
        return;
    }
    printTreeHelper(root, 0, "");
}

void HuffmanTree::printTreeHelper(const HuffmanNode* node, int indent, const std::string& prefix) const {
    if (node == nullptr) {
        return;
    }

    // Print right child first (appears at the top in vertical display)
    printTreeHelper(node->right, indent + 6, "R: ");

    // Print current node with indentation
    for (int i = 0; i < indent; ++i) {
        std::cout << " ";
    }

    std::cout << prefix;
    if (node->isLeaf()) {
        char c = node->data;
        if (c == '\n') std::cout << "'\\n' (" << node->frequency << ")" << std::endl;
        else if (c == '\t') std::cout << "'\\t' (" << node->frequency << ")" << std::endl;
        else if (c == '\r') std::cout << "'\\r' (" << node->frequency << ")" << std::endl;
        else if (c == ' ')  std::cout << "'Space' (" << node->frequency << ")" << std::endl;
        else std::cout << "'" << c << "' (" << node->frequency << ")" << std::endl;
    } else {
        std::cout << "* (" << node->frequency << ")" << std::endl;
    }

    // Print left child (appears at the bottom in vertical display)
    printTreeHelper(node->left, indent + 6, "L: ");
}
