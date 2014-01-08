## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

lib_LIBRARIES = libvx_stat_out.a
libvx_stat_out_a_SOURCES = stat_columns.cc \
              stat_hdr_columns.cc
libvx_stat_out_a_CPPFLAGS = -I${includedir}

include_HEADERS = stat_columns.h \
              stat_hdr_columns.h \
              vx_stat_out.h
