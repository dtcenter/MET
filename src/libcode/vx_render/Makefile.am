## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_render.a
libvx_render_a_SOURCES = ascii85_filter.cc \
              bit_filter.cc \
              hex_filter.cc \
              ps_filter.cc \
              psout_filter.cc \
              render_pbm.cc \
              render_pcm.cc \
              render_pgm.cc \
              render_ppm.cc \
              renderinfo.cc \
              rle_filter.cc \
              flate_filter.cc \
              uc_queue.cc
libvx_render_a_CPPFLAGS = -I${includedir} -I../vx_ps

include_HEADERS = ascii85_filter.h \
              bit_filter.h \
              hex_filter.h \
              ps_filter.h \
              psout_filter.h \
              vx_render.h \
              renderinfo.h \
              rle_filter.h \
              flate_filter.h \
              uc_queue.h
