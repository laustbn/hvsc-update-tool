// Handles MOVE and REPLACE commands. Extracted from Update.cpp
#include "MoveReplace.h"

#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;

#include "PathSplitter.h"
#include "TextFile.h"
#include "helpers.h"

bool move_replace(bool is_move, TextFile& updateFile, ErrorLogger err) {
  const fs::path hvscSource{updateFile.getLineBuf()};

  // The path we fetch the source file name from
  // if it is not a directory.
  PathSplitter sourceSplitter(hvscSource);

  // Check whether source (dir or file) exists.
  fs::path source;
  if (!getHVSCpath(source, fs::path{hvscSource})) {
    err(hvscSource.string(),
        "(source error) No such path or permission denied.",
        updateFile.getLineNum());
    // Escape from error further below, ``source'' is empty.
    // First read next line of Update script to not get
    // out of sync.
  }

  // Read destination directory/file from next line.
  updateFile.readNextLine();
  int destLine = updateFile.getLineNum();
  if (updateFile.isBlank()) {
    err(updateFile.getLineBuf(), "Premature end of update Script?",
        updateFile.getLineNum());
    return false;
  }

  // Here escape from source error.
  if (source.empty()) return false;

  // Check whether destination (dir or file) exists.
  fs::path dest;
  auto hvscDest = fs::path{updateFile.getLineBuf()};
  if (!getHVSCpath(dest, hvscDest)) {
    // Destination path does not exist.
    //
    // If it should be a directory, create it. For fs::path this means last
    // element has no filename component.
    if (!hvscDest.has_filename()) {
      makeHVSCdir(err, destLine, hvscDest.c_str());
      // Now this should not return an error.
      if (!getHVSCpath(dest, hvscDest)) {
        err(updateFile.getLineBuf(), "Creation of directory failed.", destLine);
        return false;
      }
    } else {
      // Last name in path does not end with a slash.
      // Destination file to be created is specified.
      // We leave dest.isEmpty().
    }
  }

  // Now dest exists, only if it is an existent directory
  // or file. And it is platform-specific as well.

  if (fs::is_directory(source)) {
    // Destination has to be directory as well.
    if (dest.empty() || !fs::is_directory(dest)) {
      err(dest.string(), "Destination is not a directory.", destLine);
      return false;
    }

    std::error_code ec;
    auto it = fs::directory_iterator{source, ec};
    if (ec) {
      err(source.string(), "(source error) No such path or permission denied.",
          updateFile.getLineNum());
      return false;
    }

    for (auto const& entry : it) {
      fs::path sourceFile(source);
      sourceFile /= entry.path().filename();

      if (!fs::is_directory(sourceFile))  // omit directories
      {
        // REPLACE tries to delete old version of file.
        // If old file is not there, we don`t report an error because
        // we want to get rid of it anyway.
        if (!is_move) {
          // REPLACE is case-insensitive as well.
          // Hence we seek the source file in dest dir.
          // Create HVSC-style path.
          auto tmp = fs::path(updateFile.getLineBuf());
          // Replace filename from update script with actual filename from
          // iterator. And add destination file name.
          tmp.replace_filename(entry.path().filename());
          // Determine platform-specific path and delete file if
          // available.
          fs::path removeDest;
          if (getHVSCpath(removeDest, tmp)) fs::remove(removeDest);
        }

        // Create platform-specific destination path name
        // from dest dir and source file name.
        fs::path destFile(dest);
        destFile /= (entry.path().filename());

        if (fileCopy(err, updateFile.getLineNum(), sourceFile, destFile)) {
          if (!fs::remove(sourceFile))
            err(sourceFile.string(), "Could not remove source file.",
                updateFile.getLineNum());
        }

      }  // nodir
    }  // for

  } else  // source is single file
  {       // source has platform-specific path

    // Here, ``dest'' is empty if it is a file to be created.
    // Else, it is not empty and contains a platform-specific
    // path to a file.

    // Anyway...
    // We need to seek the source file name in the
    // destination directory to be able to REPLACE
    // a file case-insensitively or trigger an error
    // (MOVE mode only).

    // Destination path to be created.
    fs::path newDestFile;
    // The file to be overwritten in REPLACE mode.
    fs::path oldDestFile;

    // Skip to source file name in source path.
    sourceSplitter.lastFile();

    // Create HVSC-style destination path.
    if (!dest.empty() && fs::is_directory(dest)) {
      // Create name of possibly existing dest.file.
      fs::path tmp(updateFile.getLineBuf());
      // Assumes slash is last char in line.
      tmp.replace_filename(sourceSplitter.getFile());
      // Determine platform-specific path.
      getHVSCpath(oldDestFile, tmp);

      // Dest.file is dest dir plus source file name.
      newDestFile.append(dest.string());
      // Append source file name.
      newDestFile.append(sourceSplitter.getFile());
    } else {
      // Although complete destination file name is given
      // in the update script, we need to determine the
      // case-insensitive path name in front of the file
      // name and the name of a possibly existing
      // destination file of different case.

      // Create HVSC-style path without the file name.
      auto tmp = fs::path(updateFile.getLineBuf());
      assert(!tmp.empty());

      // fs::path destCheck(tmp.getFile());
      // while (tmp.nextFile() && !tmp.isLastFile())
      // {
      //     destCheck.append(tmp.getFile());
      // }
      // Check whether path exists.
      if (!getHVSCpath(dest, tmp.parent_path())) {
        err(tmp.parent_path().string(),
            "(destination error) No such path or permission denied.", destLine);
        return false;
      }

      // Now see whether case-insensitive destination
      // file exists already?
      getHVSCpath(oldDestFile, tmp);

      // Complete new platform-specific path name.
      newDestFile = dest / tmp.filename();
    }

    // REPLACE tries to delete old version of file.
    // If old file is not there, we don`t report an error because
    // we want to get rid of it anyway.
    if (!is_move && !oldDestFile.empty()) {
      fs::remove(oldDestFile);
    }

    if (fileCopy(err, updateFile.getLineNum(), source, newDestFile)) {
      if (!fs::remove(source))
        err(source.string(), "Could not remove source file.",
            updateFile.getLineNum());
    }
  }  // single file

  return true;
}
