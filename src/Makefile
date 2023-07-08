# Makefile template for HVSC Update Tool.

SHELL			= /bin/sh

BASEDIR			= .
SRCDIR			= $(BASEDIR)
prefix			= /usr

CXX				= g++
CXXFLAGS		= -g -Os
RM              = /bin/rm

LDFLAGS			=
DEFS			= -DHAVE_CONFIG_H
INCLUDES		= -I$(SRCDIR) -I$(SRCDIR)/include

LIBM			= @LIBM@
LIBIOS			=
LIBS			=  $(LIBM) $(LIBIOS)

#--------------------------------------------------------------

srcdirs = . include fformat

sources = fformat/fformat_.cpp fformat/psid_.cpp fformat/pp_.cpp \
		PathCreator.cpp PathSplitter.cpp TextFile.cpp sidtune.cpp \
        FileExists.cpp hvscver.cpp Update.cpp

srcobjs = fformat/fformat_.o fformat/psid_.o fformat/pp_.o \
		PathCreator.o PathSplitter.o TextFile.o sidtune.o \
        FileExists.o hvscver.o Update.o

.EXPORT_ALL_VARIABLES:

.SUFFIXES:
.SUFFIXES: .cpp .h .o

%.o:	%.cpp
		$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFS) -c $< -o $@

Update: $(srcobjs)
		$(CXX) $(LDFLAGS) $(srcobjs) -o Update

Update.static: $(srcobjs)
		$(CXX) $(LDFLAGS) -static $(srcobjs) -o Update.static

.PHONY: all
all:	$(srcobjs) Update

.PHONY: clean
clean:
		@for subdir in $(srcdirs); do \
			(cd $$subdir && $(RM) -f *.d *.o *.bak *~ *.a *.so) || exit 1; \
		done

.PHONY: distclean
distclean:
		$(MAKE) clean
		$(RM) -f Makefile include/config.h Update Update.static

.PHONY: depend
depend:
		makedepend $(INCLUDES) $(sources) 2> /dev/null

#--------------------------------------------------------------
# DO NOT DELETE THIS LINE -- make depend depends on it.

fformat/fformat_.obj: .\include\fformat.h .\include\config.h
fformat/fformat_.obj: .\include\compconf.h .\include\mytypes.h
fformat/fformat_.obj: .\include\myendian.h
fformat/psid_.obj: fformat\psid_.h .\include\config.h .\include\mytypes.h
fformat/psid_.obj: .\include\compconf.h .\include\myendian.h
fformat/psid_.obj: .\include\sidtune.h hvscver.h
fformat/pp_.obj: fformat\pp_.h .\include\config.h .\include\mytypes.h
fformat/pp_.obj: .\include\compconf.h .\include\myendian.h
PathCreator.obj: PathCreator.h max.h
PathSplitter.obj: PathSplitter.h max.h
TextFile.obj: .\include\config.h .\include\compconf.h .\include\fformat.h
TextFile.obj: .\include\mytypes.h TextFile.h
sidtune.obj: .\include\config.h .\include\sidtune.h .\include\mytypes.h
sidtune.obj: .\include\compconf.h .\include\fformat.h .\include\myendian.h
sidtune.obj: .\include\pp.h FileExists.h
FileExists.obj: FileExists.h .\include\config.h
hvscver.obj: hvscver.h
Update.obj: .\include\config.h .\include\compconf.h FileExists.h TextFile.h
Update.obj: PathCreator.h max.h PathSplitter.h fastperc.h .\include\fformat.h
Update.obj: .\include\mytypes.h .\include\sidtune.h hvscver.h mysidtune.h
