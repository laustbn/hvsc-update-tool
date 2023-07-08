/* src/include/config.h.  Generated automatically by configure.  */
/* src/include/config.h.in.  Generated automatically from configure.in by autoheader.  */
/* Define your operating system. Leave undefined if none applies.  */
// #define HAVE_LINUX 1
/* #undef HAVE_UNIX */
#define HAVE_MSWINDOWS 1
/* #undef HAVE_MACOS */
/* #undef HAVE_OS2 */
/* #undef HAVE_AMIGAOS */
/* #undef HAVE_MSDOS */
/* #undef HAVE_BEOS */


/* Define if you support file names longer than 14 characters.  */
#define HAVE_LONG_FILE_NAMES 1

/* Define if you have the ANSI C header files.  */
/* #undef STDC_HEADERS */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define whether the compiler supports a built-in bool type.  */
#define HAVE_BOOL 1

/* Define whether Standard C++ I/O library has openmode ios::bin
   instead of ios::binary.  */
/* #undef HAVE_IOS_BIN */

/* The number of bytes in a long int.  */
#define SIZEOF_LONG_INT 4

/* Define if you have the mkdir function.  */
#define HAVE_MKDIR 1

/* Define if you have the rmdir function.  */
#define HAVE_RMDIR 1

/* Define if you have the strcasecmp function.  */
//#define HAVE_STRCASECMP 1

/* Define if you have the stricmp function.  */
#define HAVE_STRICMP 1

/* Define if you have the strncasecmp function.  */
//#define HAVE_STRNCASECMP 1

/* Define if you have the strnicmp function.  */
#define HAVE_STRNICMP 1

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the strtoul function.  */
#define HAVE_STRTOUL 1

/* Define if you have the <dirent.h> header file.  */
//#define HAVE_DIRENT_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <fstream> header file.  */
#define HAVE_FSTREAM 1

/* Define if you have the <iomanip> header file.  */
#define HAVE_IOMANIP 1

/* Define if you have the <iostream> header file.  */
#define HAVE_IOSTREAM 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <strstrea.h> header file.  */
/* #undef HAVE_STRSTREA_H */

/* Define if you have the <strstream> header file.  */
#define HAVE_STRSTREAM

/* Define if you have the <strstream.h> header file.  */
/* #undef HAVE_STRSTREAM_H */

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the <unistd.h> header file.  */
//#define HAVE_UNISTD_H 1


/* Define the file/path separator(s) that your filesystem uses:
   FS_IS_COLON_AND_BACKSLASH, FS_IS_COLON_AND_SLASH, FS_IS_BACKSLASH,
   FS_IS_COLON, FS_IS_SLASH  */
#if defined(HAVE_MSWINDOWS) || defined(HAVE_MSDOS) || defined(HAVE_OS2)
  #define FS_IS_COLON_AND_BACKSLASH
#elif defined(HAVE_MACOS)
  #define FS_IS_COLON
#elif defined(HAVE_AMIGAOS)
  #define FS_IS_COLON_AND_SLASH
#elif defined(HAVE_LINUX) || defined(HAVE_UNIX) || defined(HAVE_BEOS)
  #define FS_IS_SLASH
#else
  #error Please select an operating system in ``config.h''.
#endif	  

/* Define if standard member function ``fstream::is_open()'' is not available.  */
/* #undef DONT_HAVE_IS_OPEN */

/* Define whether istream member function ``seekg(streamoff,seekdir).offset()''
   should be used instead of standard ``seekg(streamoff,seekdir); tellg()''.  */
/* #undef HAVE_SEEKG_OFFSET */

/* --------------------------------------------------------------------------
 * Hardware-specific speed optimizations.
 * Check here for things you can configure manually only.
 * --------------------------------------------------------------------------
 *
 * Caution: This may not work on every hardware and therefore can result in
 * trouble. Some hardware-specific speed optimizations use a union to access
 * integer fixpoint operands in memory. An assumption is made about the
 * hardware and software architecture and therefore is considered a hack!
 * But try it in need for speed. You will notice if it doesn't work ;)
 *
 * This option enables direct byte-access of little/big endian values in
 * memory structures or arrays, disregarding alignment.
 * On SPARC CPUs don't define these. Else you get a ``bus error'' due
 * to 32-bit alignment of the CPU.  */
/* #undef OPTIMIZE_ENDIAN_ACCESS */
	
/* This option is highly used and tested. A failing little endian system
 * has not been reported so far.  */
/* #undef DIRECT_FIXPOINT */

/* --------------------------------------------------------------------------
 * Don't touch these!
 * --------------------------------------------------------------------------
 */
#define NO_STDIN_LOADER 1

