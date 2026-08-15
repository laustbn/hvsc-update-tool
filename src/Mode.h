#pragma once

// clang-format off
#include <optional>
#include <string>

// winnt.h defines "DELETE" breaking this. Temporarily redefine...
#pragma push_macro("DELETE")
#undef DELETE
#define DELETE DELETE

// Leave TITLE, AUTHOR, RELEASED in exact order as enumerated below.
#define MODE_ENUM_LIST(X) \
  X(TITLE)                \
  X(AUTHOR)               \
  X(RELEASED)             \
  X(SPEED)                \
  X(SONGS)                \
  X(CREDITS)              \
  X(DELETE)               \
  X(MOVE)                 \
  X(REPLACE)              \
  X(MKDIR)                \
  X(FIXLOAD)              \
  X(INITPLAY)             \
  X(MUSPLAYER)            \
  X(PLAYSID)              \
  X(CLOCK)                \
  X(SIDMODEL)             \
  X(FREEPAGES)            \
  X(FLAGS)                \
  X(COPYRIGHT)            \
  X(NO_MODE)

enum class Mode {
#define X(name) name,
  MODE_ENUM_LIST(X)
#undef X
};

[[maybe_unused]] static const std::string mode_to_string(Mode mode) {
  switch (mode) {
#define X(name) \
  case Mode::name: \
    return #name;
    MODE_ENUM_LIST(X)
#undef X
    default:
      // Shouldn't be possible
      return "UNKNOWN";
  }
}

[[maybe_unused]] static std::optional<Mode> string_to_mode(const std::string& in) {
#define X(name) \
  if (in == #name) return Mode::name;
  MODE_ENUM_LIST(X)
#undef X
  return {};
}

// This can be used in place of Mode::DELETE in other places of the code. Since
// it's constexpr, it will fail at compile time if the given string isn't a
// mode. Arguably this is still better than duplicating the definitions, IMO...
static constexpr Mode must_string_to_mode(const std::string_view& in) {
#define X(name) \
  if (in == #name) return Mode::name;
  MODE_ENUM_LIST(X)
#undef X
  throw "Invalid mode specified";
}


[[maybe_unused]] static size_t mode_to_size_t(Mode m) {
  int x = 0;
#define X(name) \
  if (m == Mode::name) return x; \
  x++;
  MODE_ENUM_LIST(X)
#undef X
  abort();
}

#pragma pop_macro("DELETE")
// clang-format on
