@echo off
:: Batch script to compile the Huffman File Compressor in C++17 on Windows.
:: It automatically checks for g++ (GCC) or cl.exe (MSVC) in the path.

echo =======================================================================
echo                 HUFFMAN FILE COMPRESSOR BUILD SCRIPT                  
echo =======================================================================
echo.

:: 1. Check for GCC (g++)
where g++ >nul 2>nul
if %errorlevel% equ 0 (
    echo [FOUND] g++ (GCC Compiler) detected.
    echo Compiling using: g++ -std=c++17 main.cpp FileManager.cpp Huffman.cpp Compressor.cpp -o huffman.exe -O2 -static
    g++ -std=c++17 main.cpp FileManager.cpp Huffman.cpp Compressor.cpp -o huffman.exe -O2 -static
    if %errorlevel% equ 0 (
        echo.
        echo [SUCCESS] Compilation complete!
        echo Run 'huffman.exe' to start the application.
    ) else (
        echo.
        echo [ERROR] Compilation failed.
    )
    goto end
)

:: 2. Check for MSVC (cl.exe)
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo [FOUND] cl (Microsoft C/C++ Compiler) detected.
    echo Compiling using: cl /EHsc /std:c++17 main.cpp FileManager.cpp Huffman.cpp Compressor.cpp /Fe:huffman.exe /O2
    cl /EHsc /std:c++17 main.cpp FileManager.cpp Huffman.cpp Compressor.cpp /Fe:huffman.exe /O2
    if %errorlevel% equ 0 (
        echo.
        echo [SUCCESS] Compilation complete!
        echo Run 'huffman.exe' to start the application.
        :: Clean up temporary build artifacts from MSVC (.obj)
        del *.obj >nul 2>nul
    ) else (
        echo.
        echo [ERROR] Compilation failed.
    )
    goto end
)

:: 3. Neither compiler found
echo [ERROR] No compatible C++17 compiler (g++ or cl.exe) was found in your PATH.
echo.
echo Suggestions:
echo 1. Install MinGW-w64 (via MSYS2) and add it to your System PATH to use 'g++'.
echo 2. Open 'Developer Command Prompt for VS' (if Visual Studio is installed) and run this build script again.
echo 3. Alternatively, install CMake and build the project using the provided CMakeLists.txt.
echo.

:end
pause
