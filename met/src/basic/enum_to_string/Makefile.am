## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS = enum_to_string
enum_to_string_SOURCES = enum_parser.yy \
              info.cc info.h \
              code.cc code.h \
              scope.cc scope.h \
              my_enum_scanner.cc  \
              enum_to_string.cc
enum_to_string_CPPFLAGS = -DCOPYRIGHT_DIR="\"${abs_top_srcdir}/data\"" ${MET_CPPFLAGS}
enum_to_string_LDFLAGS = ${MET_LDFLAGS}
enum_to_string_LDADD = -lvx_cal \
	-lvx_log

# Build enum_to_string when making a distribution so that we can
#  create all of the generated sources.

dist-hook: $(bin_PROGRAMS)

# Extra stuff needed for the distribution

EXTRA_DIST = enum.tab.h

# Clean up extra local files

CLEANFILES = enum_parser.cc \
	lex.yy.c
