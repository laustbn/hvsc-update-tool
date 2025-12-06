#include "PathSplitter.h"

#include <memory>

// This is a mostly redundant wrapper around filesystem::path to give it the
// semantics the existing code expects. The logic here should probably be folded
// into the places where it's called.
PathSplitter::PathSplitter(const std::filesystem::path* path)
    : maxPathLen(1), path(std::make_unique<std::filesystem::path>(*path)) {
  // TODO: only a pointer until we get rid of old version. Copy it rather than
  // reference because we modify it.
  // fs::path has an empty last element if path ends in slash. Trim that.
  if ((*(--path->end())).empty()) {
    this->path = std::make_unique<std::filesystem::path>((*path).parent_path());
  }
  it = ++this->path->begin();
  splitPath.ptr = 0;

  // Determine element before end(). Backwards iteration didn't work reliably
  // for me!
  while (it != this->path->end()) {
    last = it;
    it++;
  }
  firstFile();
}

// No longer called.
PathSplitter::PathSplitter(const char* pathName)
    : maxPathLen(maxCharBufferLen), path(nullptr) {
  splitPath.len = 0;

  // Allocate working buffer.
  splitPath.ptr = new char[maxPathLen];

  // Length of source path.
  int pathLen = strlen(pathName);

  if (pathLen != 0 && splitPath.ptr != 0) {
    // Convert each HVSC-style (!) directory separator into a zero.
    // At the same time save buffer offset to last dir/file name.
    int i = 0;
    int curLastSplitOffset = -1;
    while (pathName[i] != 0) {
      char c = pathName[i];
      if (c == '\\' || c == '/') {
        c = 0;
        lastSplitOffset = curLastSplitOffset;  // save previous
        curLastSplitOffset = -1;               // allow taking next
      } else {
        if (curLastSplitOffset < 0)
          curLastSplitOffset = i;  // save start of file/dir name
      }
      splitPath.ptr[i++] = c;
    }

    // Make sure we take the last file.
    if (curLastSplitOffset > lastSplitOffset)
      lastSplitOffset = curLastSplitOffset;

    splitPath.ptr[i] = 0;
    splitPath.len = pathLen;

    firstFile();
  }
}

PathSplitter::~PathSplitter(void) {
  if (splitPath.ptr != 0) delete[] splitPath.ptr;
}

const std::string PathSplitter::getFile(void) {
  if (path && it == path->end()) return std::string{};
  if (path) {
    // It seems MSVC does not automatically convert to string...
    return std::string{(*it).filename().generic_string()};
  }

  return std::string(splitPath.ptr + splitOffset);
}

bool PathSplitter::isGood(void) {
  if (path) return !path->empty();
  return (splitPath.len != 0);
}

void PathSplitter::firstFile(void) {
  if (path) {
    // Skip the first one as it is `/`
    it = ++path->begin();
    return;
  }

  splitOffset = 0;

  // Skip to next file name.
  while ((splitOffset < splitPath.len) && (splitPath.ptr[splitOffset] == 0)) {
    splitOffset++;
  };
}

void PathSplitter::lastFile(void) {
  if (path) {
    it = last;
    return;
  }

  splitOffset = lastSplitOffset;
}

bool PathSplitter::nextFile(void) {
  if (path) {
    if (it == path->end()) {
      return false;
    }
    it++;
    if (it == last || it == path->end()) return false;

    return true;
  }
  return false;  // no next file

  // Skip current file name.
  while ((splitOffset < splitPath.len) && (splitPath.ptr[splitOffset] != 0)) {
    splitOffset++;
  }

  // Skip to next file name.
  while ((splitOffset < splitPath.len) && (splitPath.ptr[splitOffset] == 0)) {
    splitOffset++;
  };

  return (splitOffset < splitPath.len);
}

bool PathSplitter::isLastFile(void) {
  if (path) {
    if (it == path->end()) return false;

    return it == last;
  }
  return (splitOffset == lastSplitOffset);
}
