## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

INCLUDES        = -I$(includedir)

bin_PROGRAMS = enum_to_string
enum_to_string_SOURCES = enum_scanner.ll enum_parser.yy \
              info.cc info.h \
              code.cc code.h \
              scope.cc scope.h \
              enum_to_string.cc
enum_to_string_LDFLAGS = -L$(libdir)
enum_to_string_LDADD = -lvx_cal

EXTRA_DIST = enum.tab.h

# Clean up extra local files
clean-local:
	-rm -f enum_scanner.cc
	-rm -f enum_parser.cc
	-rm -f lex.yy.c
