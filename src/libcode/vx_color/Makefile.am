## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_color.a
libvx_color_a_SOURCES = cfile.tab.cc \
              lex.color.cc \
              color.cc \
              color_list.cc \
              color_table.cc
libvx_color_a_CPPFLAGS = -I${includedir}

include_HEADERS = cfile.tab.h \
              color.h \
              color_list.h \
              color_parser.h \
              vx_color.h
