## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

AM_YFLAGS	= -d -p color
AM_LFLAGS	= --prefix=color --outfile=lex.yy.c

lib_LIBRARIES = libvx_color.a
libvx_color_a_SOURCES = color_parser_yacc.yy color_scanner.ll \
              color.cc \
              color_list.cc \
              color_table.cc
libvx_color_a_CPPFLAGS = -I${includedir}

include_HEADERS = color_parser_yacc.h \
              color.h \
              color_list.h \
              color_parser.h \
              vx_color.h

# Clean up extra local files
clean-local:
	-rm -f color_scanner.cc
	-rm -f color_parser_yacc.cc
	-rm -f color_parser_yacc.h
	-rm -f lex.color.cc
