## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_shapedata.a
libvx_shapedata_a_SOURCES = \
              moments.cc moments.h \
              node.cc node.h \
              shape.h \
              shapedata.cc shapedata.h \
              set.cc set.h \
              interest.cc interest.h \
              mode_columns.h \
              mode_conf_info.cc mode_conf_info.h \
              engine.cc engine.h \
              ihull.cc ihull.h \
              vx_shapedata.h
libvx_shapedata_a_CPPFLAGS = ${MET_CPPFLAGS}
