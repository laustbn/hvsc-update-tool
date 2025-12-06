#ifndef PathSplitter_h

#include <string.h>

#include <filesystem>
#include <iterator>

#include "max.h"

class PathSplitter {
  struct charBuffer {
    char* ptr;
    int len;
  };

 private:
  const int maxPathLen;

  charBuffer splitPath;

  int splitOffset;      // offset to current directory in path
  int lastSplitOffset;  // offset to last directory in path

  // New implementation
  std::unique_ptr<std::filesystem::path> path;
  std::filesystem::path::iterator it;
  std::filesystem::path::iterator last;

 public:
  PathSplitter(const char* pathName);
  PathSplitter(const std::filesystem::path* path);
  ~PathSplitter(void);

  const std::string getFile(void);

  bool isGood(void);
  bool isLastFile(void);

  void firstFile(void);
  void lastFile(void);
  bool nextFile(void);
};

#endif  // PathSplitter_h
