// Various hacks for old MSVC versions
#ifndef __config__h_
#define __config__h_

#if defined _MSC_VER

/* Define if you have the strnicmp function.  */
#define HAVE_STRNICMP 1

/* Define if you have the stricmp function.  */
#define HAVE_STRICMP 1

#else 

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strncasecmp function. */
#define HAVE_STRNCASECMP 1

#endif

// Alternatives to std::filesystem. 
#if defined(_MSC_VER) && _MSC_VER < 201703L
#include <direct.h>
#define PATH_SEPARATOR '\\'
static bool filesystem_remove(const char *name) {
    return _rmdir(name) == 0;
}

static bool filesystem_create_directory(const char *name) {
    return _mkdir(name) == 0;
}
#else
#include <filesystem>
#define PATH_SEPARATOR filesystem::path::preferred_separator

static bool filesystem_remove(const char *name) {
    return std::filesystem::remove(name);
}

static bool filesystem_create_directory(const char *name) {
    return std::filesystem::create_directory(name);
}

#endif

#endif // __config__h_
