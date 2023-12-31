//
// /home/ms/source/sidplay/libsidplay/include/RCS/pp.h,v
//

#ifndef PP_H
#define PP_H


#include "config.h"

#include <fstream>
using std::ifstream;
#include "mytypes.h"

extern bool depp(ifstream& inputFile, ubyte** destBufRef);
extern bool ppIsCompressed();
extern udword ppUncompressedLen();
extern const char* ppErrorString;


#endif
