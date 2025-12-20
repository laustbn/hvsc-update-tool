#pragma once

#include <filesystem>
#include <string>

class PathSplitter {
 private:
  std::filesystem::path path;
  std::filesystem::path::iterator it;
  std::filesystem::path::iterator last;

 public:
  PathSplitter(const std::filesystem::path path);

  const std::string getFile();

  bool isGood();
  bool isLastFile();

  void firstFile();
  void lastFile();
  bool nextFile();
};
