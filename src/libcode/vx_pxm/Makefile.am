## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_pxm.a
libvx_pxm_a_SOURCES = \
              pbm.cc pbm.h \
              pcm.cc pcm.h \
              pgm.cc pgm.h \
              ppm.cc ppm.h \
              pxm_base.cc pxm_base.h \
              pxm_utils.cc pxm_utils.h \
              vx_pxm.h
libvx_pxm_a_CPPFLAGS = ${MET_CPPFLAGS}
