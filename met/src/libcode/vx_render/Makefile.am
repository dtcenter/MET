## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_render.a
libvx_render_a_SOURCES = \
              ascii85_filter.cc ascii85_filter.h \
              bit_filter.cc bit_filter.h \
              hex_filter.cc hex_filter.h \
              ps_filter.cc ps_filter.h \
              psout_filter.cc psout_filter.h \
              render_pbm.cc \
              render_pcm.cc \
              render_pgm.cc \
              render_ppm.cc \
              renderinfo.cc renderinfo.h \
              rle_filter.cc rle_filter.h \
              flate_filter.cc flate_filter.h \
              uc_queue.cc uc_queue.h \
              vx_render.h
libvx_render_a_CPPFLAGS = ${MET_CPPFLAGS}
