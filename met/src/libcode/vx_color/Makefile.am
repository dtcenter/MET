## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# YACC/LEX flags

#AM_YFLAGS	= -d -p color
AM_YFLAGS	= --defines=color_parser_yacc.h -p color
AM_LFLAGS	= --prefix=color --outfile=lex.yy.c

# The library

noinst_LIBRARIES = libvx_color.a
libvx_color_a_SOURCES = \
              color_parser_yacc.yy color_scanner.ll \
              color.cc color.h \
              color_list.cc color_list.h \
              color_table.cc \
              color_parser.h \
              vx_color.h
libvx_color_a_CPPFLAGS = ${MET_CPPFLAGS}

# Clean up extra local files

CLEANFILES = color_scanner.cc \
	color_parser_yacc.cc \
	color_parser_yacc.h \
	lex.color.cc
