#pragma once

// Classes for reading Update.hvs and hvsids.txt

#include <cstddef>
#include <fstream>
#include <optional>

using namespace std;

// Reads the update.hvs script. Pos() and Size() can be used to determine
// progress. These track overall read progress in bytes as we don't know the
// number of lines. This is *only* intended for processing commands and
// automatically strips comments and blanks. The header parsing is handled by
// HeaderReader.
struct UpdateReader {
  UpdateReader(const std::string &fileName);

  // Read next line from the file. Returns false on last line and subsequent
  // calls.
  bool NextLine();

  // Returns current line. At end of file, this will repeatedly return the last
  // line.
  std::string GetLine();

  int GetLineNum();

  // Current read position
  size_t Pos();

  // Total file size.
  size_t Size();

 private:
  // Current line
  string lineBuf;

  // Total size (in bytes)
  size_t size = 0;

  // Current line
  int line = 0;

  ifstream inFile;
};

struct HeaderReader {
  HeaderReader(const std::string &fileName);

  // Read next line from the file. Returns false on last line and subsequent
  // calls.
  bool NextLine();

  // Check whether the first characters are equal to a given keyword string.
  // If true, returns remainder of the line.
  std::optional<std::string> FindKey(const std::string &key);

  int GetLineNum();

 private:
  ifstream inFile;
  string lineBuf;
  int line = 0;
};
