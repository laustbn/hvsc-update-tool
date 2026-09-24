#pragma once
#include <string>

using HVSCVER = unsigned int;

#define MAKE_HVSCVER(major, minor) ((major * 10) + (minor % 10))
#define HVSCVER_MAJOR(version) (version / 10)
#define HVSCVER_MINOR(version) (version % 10)

extern bool atohvscver(const std::string &s_version, HVSCVER &hvscver);
extern std::string hvscvertoa(HVSCVER);
extern int hvscvercmp(HVSCVER, HVSCVER);
