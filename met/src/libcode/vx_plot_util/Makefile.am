## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

noinst_LIBRARIES = libvx_plot_util.a
libvx_plot_util_a_SOURCES = \
              map_region.cc map_region.h \
              vx_plot_util.cc vx_plot_util.h \
              data_plane_plot.cc data_plane_plot.h
libvx_plot_util_a_CPPFLAGS = ${MET_CPPFLAGS}
