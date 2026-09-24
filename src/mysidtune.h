#pragma once
#include "Mode.h"
#include "hvscver.h"
#include "sidtune.h"

static const int maxSidInfoLen = 32;  // not including terminator

class mySidTune : public sidTune {
 public:
  mySidTune(const char* fileName, HVSCVER hvscVersion);

  bool writeToSidTune(const std::array<std::string, 4>& newInfoString,
                      Mode mode);
};
