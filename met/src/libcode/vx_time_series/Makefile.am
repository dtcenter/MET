## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_time_series.a
libvx_time_series_a_SOURCES = \
               compute_swinging_door.cc compute_swinging_door.h \
               time_series_util.cc time_series_util.h \
               vx_time_series.h
libvx_time_series_a_CPPFLAGS = ${MET_CPPFLAGS}
