## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_pxm.a
libvx_pxm_a_SOURCES = pbm.cc \
              pcm.cc \
              pgm.cc \
              ppm.cc \
              pxm_base.cc \
              pxm_utils.cc
libvx_pxm_a_CPPFLAGS = -I${includedir}

include_HEADERS = pbm.h \
              pcm.h \
              pgm.h \
              ppm.h \
              pxm_base.h \
              pxm_utils.h \
              vx_pxm.h
