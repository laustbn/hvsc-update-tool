#pragma once
#include <filesystem>
#include <functional>

// generic interface for logging an error
using ErrorLogger = std::function<void(std::string lineContent,
                                       std::string errorMessage, int lineNum)>;

bool getHVSCpath(std::filesystem::path &outPath,
                 const std::filesystem::path &srcPath, const int debug = 0);

bool makeHVSCdir(ErrorLogger err, int line, std::filesystem::path hvscPath);

bool fileCopy(ErrorLogger err, int line, std::filesystem::path inFileName,
              std::filesystem::path outFileName);

#if defined(_MSC_VER)
#define DEBUGGER __debugbreak()
#elif defined(__CLANG__)
#define DEBUGGER __builtin_debugtrap()
#elif defined(__GNUC__)
#define DEBUGGER __builtin_trap()
#endif
