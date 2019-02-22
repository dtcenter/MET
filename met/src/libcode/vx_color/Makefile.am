## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# YACC/LEX flags

AM_YFLAGS	= --defines=color_parser_yacc.h -p color
#AM_LFLAGS	= --prefix=color --outfile=lex.yy.c

# The library

noinst_LIBRARIES = libvx_color.a

if ENABLE_DEVELOPMENT
    libvx_color_a_SOURCES = color_parser_yacc.yy my_color_scanner.cc
else
    libvx_color_a_SOURCES = color_parser_yacc.cc color_parser_yacc.h my_color_scanner.cc
endif

libvx_color_a_SOURCES += \
              color.cc color.h \
              color_list.cc color_list.h \
              color_table.cc \
              color_parser.h \
              my_color_scanner.cc \
              vx_color.h
libvx_color_a_CPPFLAGS = ${MET_CPPFLAGS}

# If we are in development mode, clean up extra local files in a "make clean".

if ENABLE_DEVELOPMENT
    CLEANFILES = color_scanner.cc \
        	color_parser_yacc.cc \
        	color_parser_yacc.h \
        	lex.color.cc
endif
