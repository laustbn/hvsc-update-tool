// Various helper functions

#include "helpers.h"

// for old implementation
#include <filesystem>
#include <iostream>
#include <string>

#include "PathSplitter.h"

namespace fs = std::filesystem;
using namespace std;

static const char PARENT_DIR[] = "..";

bool fileCopy(ErrorLogger err, int line, fs::path inFileName,
              fs::path outFileName) {
  if (inFileName.empty()) {
    err(inFileName.string(), "Malformed input filename", 0);
    return false;
  }
  if (outFileName.empty()) {
    err(outFileName.string(), "Malformed output filename", 0);
    return false;
  }

  // TODO: check if options need to be changed
  if (!fs::copy_file(inFileName, outFileName)) {
    err(outFileName.string(), "Copy failed", line);
    return false;
  }
  return true;
}

// Build a local path for outPath that honors local casing of files and
// directories, e.g. Given the input /DEMOS/A-F/Foo.sid and a local directory
// cased "demOS" and a file named `foo.sid`, this should return
// `../demOS/A-F/foo.sid`. When this is executed, we're in `updates/`, so `../`
// is automatically prefixed as well.
//
// outPath will be empty unless the entire input path was found.
//
// Does not handle inputs with backslashes (a breaking change). No such paths
// have occurred in updates since update 7.
//
// --------------------------------------------------------------------------
// Old description for reference
//
// Function searches HVSC tree for the specified directory or file passed
// in "path" without regard to upper or lower case, starting in parent
// directory. The correct case of the file or directory is determined and
// created in "outPath". If the path could not be found, false is returned
// and "outPath" is empty.
//
// - Ignores leading slash.
// - Starts searching in parent directory.
// - Does not work if source path contains platform-specific ".."
//   directories.
//
// Input: HVSC-style path (.., / or \)
// Output: platform-specific path
bool getHVSCpath(fs::path &outPath, const fs::path &srcPath, const int debug) {
  outPath.clear();
  outPath.append("..");
  const bool checkAmbiguous = false;

  if (debug > 0) cout << "Starting to find " << srcPath << endl;

  for (const auto &p : srcPath) {
    if (debug > 0) cout << "Processing element " << p << endl;

    // Root is the first element and we don't need it
    if (!p.has_filename()) {
      continue;
    }

    // Iterate all files in this directory
    const auto search = outPath;
    auto lc = to_lower(p.filename().string());

    if (!fs::exists(search)) {
      if (debug > 0) cout << "Not found, pre-iteration " << p << endl;
      outPath.clear();
      return false;
    }

    bool found = false;
    for (auto const &dir_entry : std::filesystem::directory_iterator{search}) {
      if (debug > 1)
        cout << "Checking " << dir_entry.path().filename() << ", " << lc
             << endl;

      if (to_lower(dir_entry.path().filename().string()) == lc) {
        if (checkAmbiguous && found) {
          // If multiple entries match input, it is
          // ambiguous. Old code didn't care about this.
        }
        found = true;
        outPath /= dir_entry.path().filename();
        if (debug > 0)
          cout << "Found, appending " << dir_entry.path().filename() << endl;
        // TODO: Sanity check: unless this is the last element of srcPath,
        // it must be a directory.
      }
    }

    if (!found) {
      if (debug > 0)
        cout << "Not found post-iteration " << (outPath / p.filename()) << endl;
      outPath.clear();
      return false;
    }
  }

  if (debug > 0) cout << "Succeeded, final path " << outPath << endl;

  return true;
}

bool makeHVSCdir(ErrorLogger err, int line, fs::path hvscPath) {
  // Summary:
  //
  // If path already exists, ignore MKDIR request.
  // If path points to a file, log appropriate error.
  //
  // If path does not exist, create last directory in path
  // if this ought to be a directory.

  fs::path tmpDest;
  // Check whether path exists.
  if (!getHVSCpath(tmpDest, hvscPath))  // path not found?
  {
    // We use this to analyze the input path.
    PathSplitter myPathSplitter(hvscPath);

    // TODO: verify input beforehand to fail earlier make this check redundant
    if (!myPathSplitter.isGood()) {
      DEBUGGER;
      err(hvscPath.string(), "Bad input.", line);
      return false;
    }

    // The path we will construct.
    fs::path dest;

    // Construct HVSC-style path.
    // dest = system-specific path, tmpDest = HVSC-style path

    // Special case: top-level directory only. Parent does exist
    // (= HVSC root), so we don`t need to check it.
    if (myPathSplitter.isLastFile()) {
      // Not HVSC-style!
      dest.append(PARENT_DIR);
      dest.append(myPathSplitter.getFile());
    } else {
      tmpDest.clear();  // redundant
      tmpDest.append(myPathSplitter.getFile());
      // Omit last file/dir, so we can check the rest of the path.
      while (myPathSplitter.nextFile() && !myPathSplitter.isLastFile()) {
        tmpDest.append(myPathSplitter.getFile());
      }

      // Check whether contructed path exists.
      if (!getHVSCpath(dest, tmpDest))  // path not found?
      {
        err(tmpDest.string(), "Path does not exist.", line);
        return false;
      }

      // Append directory to create.
      myPathSplitter.lastFile();
      dest.append(myPathSplitter.getFile());
    }

    bool success = fs::create_directory(dest);
    if (!success) {
      err(dest.string(), "Could not create directory.", line);
    }
    return success;
  } else  // path found
  {
    bool success = fs::is_directory(tmpDest);
    if (!success) {
      err(tmpDest.string(), "Trying to create directory on top of file.", line);
    }
    return success;
  }
}

std::string to_lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}
