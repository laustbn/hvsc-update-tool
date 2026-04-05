#pragma once
#include "sidtune.h"
#include "hvscver.h"
#include "Mode.h"

static const int maxSidInfoLen = 32;   // not including terminator

class mySidTune : public sidTune {
 public:
  mySidTune(const char* fileName, HVSCVER hvscVersion);

  bool writeToSidTune(char newInfoString[][maxSidInfoLen + 1], Mode mode);
};
