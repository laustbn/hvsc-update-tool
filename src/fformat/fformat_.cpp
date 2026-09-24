//
// /home/ms/source/sidplay/libsidplay/fformat/RCS/fformat_.cpp,v
//

#include <ctype.h>
#include <string.h>

#include <filesystem>

#include "fformat.h"
#include "myendian.h"

// Return pointer to file name position in complete path.
char* fileNameWithoutPath(char* s) {
#if _MSVC_LANG >= 201703L
  const char sep = std::filesystem::path::preferred_separator;
#else
  const char sep = '\\';
#endif
  for (size_t pos = strlen(s); pos > 0; pos--) {
    if (s[pos] == sep) {
      return (&s[pos + 1]);
    }
  }
  return 0;
}

// Parse input string stream. Read and convert a decimal number up
// to a ``,'' or ``:'' or ``\0'' or end of stream.
udword readDec(istringstream& decin) {
  udword hexLong = 0;
  char c;
  do {
    decin >> c;
    if (!decin) break;
    if ((c != ',') && (c != ':') && (c != 0)) {
      c &= 0x0f;
      hexLong *= 10;
      hexLong += (udword)c;
    } else {
      if (c == 0) decin.putback(c);
      break;
    }
  } while (decin);
  return hexLong;
}

// Search terminated string for next newline sequence.
// Skip it and return pointer to start of next line.
const char* returnNextLine(const char* s) {
  // Unix: LF = 0x0A
  // Windows, DOS: CR,LF = 0x0D,0x0A
  // Mac: CR = 0x0D
  char c;
  while ((c = *s) != 0) {
    s++;  // skip read character
    if (c == 0x0A) {
      break;  // LF found
    } else if (c == 0x0D) {
      if (*s == 0x0A) {
        s++;  // CR,LF found, skip LF
      }
      break;  // CR or CR,LF found
    }
  }
  if (*s == 0)  // end of string ?
  {
    return 0;  // no next line available
  }
  return s;  // next line available
}

// Skip any characters in an input string stream up to '='.
void skipToEqu(istringstream& parseStream) {
  char c;
  do {
    parseStream >> c;
  } while (c != '=');
}

void copyStringValueToEOL(const char* pSourceStr, char* pDestStr,
                          int DestMaxLen) {
  // Start at first character behind '='.
  while (*pSourceStr != '=') {
    pSourceStr++;
  }
  pSourceStr++;  // Skip '='.
  while ((DestMaxLen > 0) && (*pSourceStr != 0) && (*pSourceStr != '\n') &&
         (*pSourceStr != '\r')) {
    *pDestStr++ = *pSourceStr++;
    DestMaxLen--;
  }
  *pDestStr++ = 0;
}
