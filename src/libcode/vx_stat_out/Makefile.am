## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_stat_out.a
libvx_stat_out_a_SOURCES = \
              stat_columns.cc stat_columns.h \
              stat_hdr_columns.cc stat_hdr_columns.h \
              vx_stat_out.h
libvx_stat_out_a_CPPFLAGS = ${MET_CPPFLAGS}
