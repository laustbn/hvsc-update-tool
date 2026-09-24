// Various hacks for old MSVC versions
#ifndef __config__h_
#define __config__h_

#ifndef _MSC_VER

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strncasecmp function. */
#define HAVE_STRNCASECMP 1

#endif

#endif  // __config__h_
