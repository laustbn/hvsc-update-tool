#pragma once

#include <optional>
#include <string>

// Leave TITLE, AUTHOR, RELEASED in exact order as enumerated below.
#define MODE_ENUM_LIST \
  X(TITLE)             \
  X(AUTHOR)            \
  X(RELEASED)          \
  X(SPEED)             \
  X(SONGS)             \
  X(CREDITS)           \
  X(DELETE)            \
  X(MOVE)              \
  X(REPLACE)           \
  X(MKDIR)             \
  X(FIXLOAD)           \
  X(INITPLAY)          \
  X(MUSPLAYER)         \
  X(PLAYSID)           \
  X(CLOCK)             \
  X(SIDMODEL)          \
  X(FREEPAGES)         \
  X(FLAGS)             \
  X(COPYRIGHT)         \
  X(NO_MODE)

enum class Mode {
#define X(name) name,
  MODE_ENUM_LIST
#undef X
};

static const std::string mode_to_string(Mode mode) {
  switch (mode) {
#define X(name)    \
  case Mode::name: \
    return #name;
    MODE_ENUM_LIST
#undef X
    default:
      // Shouldn't be possible
      return "UNKNOWN";
  }
}

static std::optional<Mode> string_to_mode(const std::string& in) {
#define X(name) \
  if (in == #name) return Mode::name;
  MODE_ENUM_LIST
#undef X
  return {};
}

static int mode_to_int(Mode m) {
  int x = 0;
#define X(name)                  \
  if (m == Mode::name) return x; \
  x++;
  MODE_ENUM_LIST
#undef X
  abort();
}
