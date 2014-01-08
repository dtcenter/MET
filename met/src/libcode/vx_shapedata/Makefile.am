## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_shapedata.a
libvx_shapedata_a_SOURCES = moments.cc \
              node.cc \
              polyline.cc \
              shapedata.cc \
              set.cc \
              interest.cc \
              mode_conf_info.cc \
              engine.cc
libvx_shapedata_a_CPPFLAGS = -I${includedir}

include_HEADERS = moments.h \
              node.h \
              polyline.h \
              shape.h \
              shapedata.h \
              set.h \
              interest.h \
              mode_columns.h \
              mode_conf_info.h \
              engine.h \
              vx_shapedata.h
