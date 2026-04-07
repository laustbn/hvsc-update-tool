//
// /home/ms/source/sidplay/libsidplay/include/RCS/fformat.h,v
//

#ifndef fformat_h
#define fformat_h

#include <sstream>

#include "mytypes.h"
using std::istringstream;

// Return pointer to file name position in complete path.
extern char* fileNameWithoutPath(char* s);

// Parse input string stream. Read and convert a decimal number up
// to a ``,'' or ``:'' or ``\0'' or end of stream.
extern udword readDec(istringstream& parseStream);

// Search terminated string for next newline sequence.
// Skip it and return pointer to start of next line.
extern const char* returnNextLine(const char* pBuffer);

// Skip any characters in an input string stream up to '='.
extern void skipToEqu(istringstream& parseStream);

// Start at first character behind '=' and copy rest of string.
extern void copyStringValueToEOL(const char* pSourceStr, char* pDestStr,
                                 int destMaxLen);

#endif  // fformat_h
