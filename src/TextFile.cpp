#include "TextFile.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

#include "helpers.h"

// New implementation

UpdateReader::UpdateReader(const std::string &fileName) : inFile(fileName) {
  if (!inFile) {
    throw std::runtime_error("Failed to open file: " + fileName);
  }
  // Breaks > 4GB which we accept as a limitation
  size = static_cast<size_t>(std::filesystem::file_size(fileName));
}

// Check whether the first non-space character is a ``#'' or ``;''.
static bool isComment(const std::string &buf) {
  return ((buf[0] == ';') || (buf[0] == '#'));
}

// Check whether the line is blank (assumes spaces have been stripped already).
static bool isBlank(const std::string &buf) {
  return buf.length() == 0;
  ;
}

bool UpdateReader::NextLine() {
  std::string s;
  while (getline(inFile, s)) {
    line += 1;

    // Trim trailing whitespace. It makes parsing better and also handles
    // trailing \r on UNIX systems.
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
      s.pop_back();
    }

    if (isComment(s) || isBlank(s)) {
      continue;
    }
    lineBuf = s;
    return true;
  }
  inFile.close();
  return false;
}

size_t UpdateReader::Pos() { return static_cast<size_t>(inFile.tellg()); }

int UpdateReader::GetLineNum() { return line; }

std::string UpdateReader::GetLine() { return lineBuf; }

size_t UpdateReader::Size() { return size; }

HeaderReader::HeaderReader(const std::string &fileName) : inFile(fileName) {
  if (!inFile) {
    throw std::runtime_error("Failed to open file: " + fileName);
  }
}

bool HeaderReader::NextLine() {
  std::string tmp;
  while (getline(inFile, tmp)) {
    // Remove all whitespace
    tmp.erase(std::remove_if(tmp.begin(), tmp.end(),
                             [](unsigned char c) { return std::isspace(c); }),
              tmp.end());

    lineBuf = tmp;
    line++;
    return true;
  }
  return false;
}

int HeaderReader::GetLineNum() { return line; }

std::optional<std::string> HeaderReader::FindKey(const std::string &key) {
  // Case insensitive
  auto lc = to_lower(lineBuf);
  if (key.size() <= lc.size() &&
      lc.compare(0, key.size(), to_lower(key)) == 0) {
    // Could probably be optimized to a string_view...
    return lineBuf.substr(key.length());
  }
  return {};
}
