cmake_minimum_required(VERSION 3.10)
project(HuffmanCompressor VERSION 1.0.0 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure compiler flags (warnings)
if(MSVC)
    add_compile_options(/W4 /EHsc)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Source files
set(SOURCES
    main.cpp
    FileManager.h
    FileManager.cpp
    Huffman.h
    Huffman.cpp
    Compressor.h
    Compressor.cpp
)

# Define executable
add_executable(HuffmanCompressor ${SOURCES})

# Include directory setup
target_include_directories(HuffmanCompressor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
