#include "hvscver.h"

#include <sstream>

bool atohvscver(const std::string &s_version, HVSCVER &hvscver) {
  unsigned int n_major = 0;
  unsigned int n_minor = 0;
  char dot = 0;

  std::istringstream iss(s_version);

  iss >> n_major >> dot >> n_minor;
  if (n_major <= 0) {
    return false;
  }

  hvscver = MAKE_HVSCVER(n_major, n_minor);
  return true;
}

std::string hvscvertoa(HVSCVER hvscver) {
  std::ostringstream oss;
  oss << HVSCVER_MAJOR(hvscver) << '.' << HVSCVER_MINOR(hvscver);
  return oss.str();
}

int hvscvercmp(HVSCVER a, HVSCVER b) {
  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  }
  return 0;
}
