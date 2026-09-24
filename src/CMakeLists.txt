add_library(libupdate)

target_include_directories(libupdate PUBLIC "include" ".")
target_sources(libupdate PRIVATE
  PathSplitter.cpp
  MoveReplace.cpp
  TextFile.cpp
  hvscver.cpp
  sidtune.cpp
  helpers.cpp
  mysidtune.cpp
  fformat/fformat_.cpp
  fformat/pp_.cpp
  fformat/psid_.cpp
)
