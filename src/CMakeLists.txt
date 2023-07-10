add_library(libupdate)

target_include_directories(libupdate PUBLIC "include" ".")
target_sources(libupdate PRIVATE
  FileExists.cpp
  PathCreator.cpp
  PathSplitter.cpp
  TextFile.cpp
  hvscver.cpp
  sidtune.cpp
  fformat/fformat_.cpp
  fformat/pp_.cpp
  fformat/psid_.cpp
)
