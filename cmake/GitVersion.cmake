find_package(Git QUIET)

set(GIT_VERSION "nogit")
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}" rev-parse --short=12 HEAD
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  if(GIT_HASH)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_SOURCE_DIR}" status --porcelain
      OUTPUT_VARIABLE GIT_STATUS
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )

    set(GIT_VERSION "${GIT_HASH}")
    if(NOT GIT_STATUS STREQUAL "")
      string(APPEND GIT_VERSION "-dirty")
    endif()
  endif()
endif()

# Reconfigure triggers
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/.git/HEAD"
)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
  file(READ "${CMAKE_SOURCE_DIR}/.git/HEAD" GIT_HEAD_CONTENT)
  string(STRIP "${GIT_HEAD_CONTENT}" GIT_HEAD_CONTENT)
  if(GIT_HEAD_CONTENT MATCHES "^ref: (.+)$")
    set(GIT_HEAD_REF "${CMAKE_MATCH_1}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
      "${CMAKE_SOURCE_DIR}/.git/${GIT_HEAD_REF}"
    )
  endif()
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/.git/packed-refs")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/packed-refs"
  )
endif()
