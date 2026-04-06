#include "mysidtune.h"

#include <charconv>
#include <system_error>

#include "Mode.h"
#include "hvscver.h"

// Only derive the simple constructor.
mySidTune::mySidTune(const char* fileName, HVSCVER hvscVersion)
    : sidTune(fileName, hvscVersion) {};

unsigned long my_strtoul(const std::string& input, int base) {
  unsigned long value = 0;
  auto result =
      std::from_chars(input.data(), input.data() + input.size(), value, base);
  if (result.ec == std::errc()) {
    return value;
  }
  // Errors silently ignored
  return 0;
}

bool mySidTune::writeToSidTune(const std::array<std::string, 4>& newInfoString,
                               Mode mode) {
  // PSID-format can only handle up to 31 characters plus a terminating zero.
  //
  // The infoStrings are kept without any special order in a two-dimensional
  // array of the size [infoStringNum][infoStringLen].
  //
  // To make available publically readable copies for each infoString,
  // extra pointers are assigned: info.nameString, info.authorString,
  // info.copyrightString, and for compatibility to MUS files and a future
  // file format: info.infoString[0], ..., info.infoString[infoStringNum].
  // A copy of the private instance of the sidTuneInfo structure can be read
  // out using ::returnInfo().

  int infoStringIndex = 0;  // Used for FLAGS case.

  switch (mode) {
    case Mode::TITLE:
    case Mode::AUTHOR:
    case Mode::RELEASED: {
      // Copy string to private array.
      infoString[(int)mode] = newInfoString[(int)mode];

      info.infoString[(int)mode] = &infoString[(int)mode][0];

      if (Mode::TITLE == mode)
        // Assign pointer: First infoString usually is NAME.
        info.nameString = infoString[(int)mode][0];
      else if (Mode::AUTHOR == mode)
        // Assign pointer: Second infoString usually is AUTHOR.
        info.authorString = infoString[(int)mode][0];
      else
        // Assign pointer: Third infoString usually is RELEASED
        // (original release information).
        info.copyrightString = infoString[(int)mode][0];
      break;
    }

    case Mode::CREDITS: {
      if ('*' != newInfoString[0][0]) {
        infoString[0] = newInfoString[0];
        info.nameString = infoString[0];
        info.infoString[0] = infoString[0];
      }
      if ('*' != newInfoString[1][0]) {
        infoString[1] = newInfoString[1];
        info.authorString = infoString[1];
        info.infoString[1] = infoString[1];
      }
      if ('*' != newInfoString[2][0]) {
        infoString[2] = newInfoString[2];
        info.copyrightString = infoString[2];
        info.infoString[2] = infoString[2];
      }
      break;
    }

    case Mode::SPEED: {
      unsigned long ulSpeed;

      // Only 32 song speed can be set with the below code assuming
      // an unsigned long stores 32 bits.
      if (sizeof(ulSpeed) >= 4) {
        // Not modifiable!
        if (info.compatibility == SIDTUNE_COMPATIBILITY_R64) return false;
        // SPEED string is in hex.
        ulSpeed = my_strtoul(newInfoString[0], 16);
        convertOldStyleSpeedToTables((udword)ulSpeed);
        break;
      } else
        return false;
    }

    case Mode::SONGS:  // SONGS string must have a comma!
    {
      // Split string at the comma
      // example, with SONGS=3,2 the string is "3"
      auto index = newInfoString[0].find(",");

      // If a comma found, continue...else fail
      if (index == std::string::npos) {
        return false;
      }

      auto songs = newInfoString[0].substr(0, index);
      auto start = newInfoString[0].substr(index + 1);

      info.songs = my_strtoul(songs, 10);
      info.startSong = my_strtoul(start, 10);
      break;
    }

    case Mode::INITPLAY:  // INITPLAY string must have a comma!
    {
      // Split string at the comma
      // example, with INITPLAY=1000,1003 the string is "1000"
      auto index = newInfoString[0].find(",");

      // If a comma found, continue...else fail
      if (index == std::string::npos) {
        return false;
      }

      auto init = newInfoString[0].substr(0, index);
      auto play = newInfoString[0].substr(index + 1);

      // The strings are in hex. Note that in RSID mode certain
      // values of init are illegal.  This is not checked here
      info.initAddr = (uword)my_strtoul(init, 16);
      info.playAddr = (uword)my_strtoul(play, 16);

      // Not modifiable!
      if (checkCompatibility() == false) return false;
      break;
    }

    case Mode::FREEPAGES:  // FREEPAGES string must have a comma!
    {
      // Split string at the comma
      // example, with FREEPAGES=20,03 the string is "20"
      auto index = newInfoString[0].find(",");

      // If a comma found, continue...else fail
      if (index == std::string::npos) {
        return false;
      }

      auto start = newInfoString[0].substr(0, index);
      auto pages = newInfoString[0].substr(index + 1);

      info.relocStartPage = (ubyte)my_strtoul(start, 16);
      info.relocPages = (ubyte)my_strtoul(pages, 16);
      if (checkRelocInfo() == false) return false;
      break;
    }

    case Mode::FLAGS:  // We'll fall thru the next 4 cases for this one.
    case Mode::MUSPLAYER: {
      if ((mode == Mode::FLAGS) && (newInfoString[infoStringIndex][0] == '*')) {
        ;  // Do nothing - this field is not to be changed.
      } else if (my_strtoul(newInfoString[infoStringIndex], 10) == 0) {
        info.musPlayer = false;
      } else if (my_strtoul(newInfoString[infoStringIndex], 10) == 1) {
        info.musPlayer = true;
      } else {
        return false;
      }

      if (mode != Mode::FLAGS) {
        break;
      } else {
        // Fall through.
        infoStringIndex++;
      }
    }

    case Mode::PLAYSID: {
      if ((mode == Mode::FLAGS) && (newInfoString[infoStringIndex][0] == '*')) {
        ;  // Do nothing - this field is not to be changed.
      } else if (my_strtoul(newInfoString[infoStringIndex], 10) == 0) {
        if ((info.compatibility != SIDTUNE_COMPATIBILITY_C64) &&
            (info.compatibility != SIDTUNE_COMPATIBILITY_PSID)) {
          return false;
        }
        info.compatibility = SIDTUNE_COMPATIBILITY_C64;
      } else if (my_strtoul(newInfoString[infoStringIndex], 10) == 1) {
        if ((info.compatibility != SIDTUNE_COMPATIBILITY_C64) &&
            (info.compatibility != SIDTUNE_COMPATIBILITY_PSID)) {
          return false;
        }
        info.compatibility = SIDTUNE_COMPATIBILITY_PSID;
      } else {
        return false;
      }

      if (mode != Mode::FLAGS) {
        break;
      } else {
        // Fall through.
        infoStringIndex++;
      }
    }

    case Mode::CLOCK: {
      if ((mode == Mode::FLAGS) && (newInfoString[infoStringIndex][0] == '*')) {
        ;  // Do nothing - this field is not to be changed.
      } else if (newInfoString[infoStringIndex] == "UNKNOWN") {
        info.clockSpeed = SIDTUNE_CLOCK_UNKNOWN;
      } else if (newInfoString[infoStringIndex] == "PAL") {
        info.clockSpeed = SIDTUNE_CLOCK_PAL;
      } else if (newInfoString[infoStringIndex] == "NTSC") {
        info.clockSpeed = SIDTUNE_CLOCK_NTSC;
      } else if (newInfoString[infoStringIndex] == "ANY" ||
                 newInfoString[infoStringIndex] == "EITHER") {
        info.clockSpeed = SIDTUNE_CLOCK_ANY;
      } else {
        return false;
      }

      if (mode != Mode::FLAGS) {
        break;
      } else {
        // Fall through.
        infoStringIndex++;
      }
    }

    case Mode::SIDMODEL: {
      if ((mode == Mode::FLAGS) && (newInfoString[infoStringIndex][0] == '*')) {
        ;  // Do nothing - this field is not to be changed.
      } else if (newInfoString[infoStringIndex] == "UNKNOWN") {
        info.sidModel = SIDTUNE_SIDMODEL_UNKNOWN;
      } else if (newInfoString[infoStringIndex] == "6581") {
        info.sidModel = SIDTUNE_SIDMODEL_6581;
      } else if (newInfoString[infoStringIndex] == "8580") {
        info.sidModel = SIDTUNE_SIDMODEL_8580;
      } else if (newInfoString[infoStringIndex] == "ANY" ||
                 newInfoString[infoStringIndex] == "EITHER") {
        info.sidModel = SIDTUNE_SIDMODEL_ANY;
      } else {
        return false;
      }

      break;  // The FLAGS directive stops here, too.
    }

    case Mode::FIXLOAD: {
      // Increase load address by 2 without verification.
      fixLoadAddress(true, info.initAddr, info.playAddr);
      break;
    }

    default: {
      return false;
    }
  }  // switch

  return true;

};  // writeToSidTune
