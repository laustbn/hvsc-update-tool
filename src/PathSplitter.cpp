#include "PathSplitter.h"

#include <filesystem>
#include <string>

// This is a mostly redundant wrapper around filesystem::path to give it the
// semantics the existing code expects. The logic here should probably be folded
// into the places where it's called.
PathSplitter::PathSplitter(const std::filesystem::path path) : path(path) {
  // TODO: only a pointer until we get rid of old version. Copy it rather than
  // reference because we modify it.
  // fs::path has an empty last element if path ends in slash. Trim that.
  if ((*(--path.end())).empty()) {
    this->path = path.parent_path();
  }
  it = ++this->path.begin();

  // Determine element before end(). Backwards iteration didn't work reliably
  // for me!
  while (it != this->path.end()) {
    last = it;
    it++;
  }
  firstFile();
}

const std::string PathSplitter::getFile() {
  if (it == path.end()) return std::string{};

  // It seems MSVC does not automatically convert to string...
  return std::string{(*it).filename().generic_string()};
}

bool PathSplitter::isGood() { return !path.empty(); }

void PathSplitter::firstFile() {
  // Skip the first one as it is `/`
  it = ++path.begin();
  return;
}

void PathSplitter::lastFile() { it = last; }

bool PathSplitter::nextFile() {
  if (it == path.end()) {
    return false;
  }
  it++;
  if (it == last || it == path.end()) return false;

  return true;
}

bool PathSplitter::isLastFile() {
  if (it == path.end()) return false;

  return it == last;
}
