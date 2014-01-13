## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Test programs

noinst_PROGRAMS = test_map_region

test_map_region_SOURCES = test_map_region.cc
test_map_region_CPPFLAGS = -I${includedir}
test_map_region_LDFLAGS = -L${libdir}
test_map_region_LDADD = -lvx_plot_util \
	-lvx_util \
	-lvx_log \
	-lm

# The library itself

lib_LIBRARIES = libvx_plot_util.a
libvx_plot_util_a_SOURCES = map_region.cc \
              vx_plot_util.cc \
              data_plane_plot.cc
libvx_plot_util_a_CPPFLAGS = -I${includedir}

include_HEADERS = map_region.h \
              vx_plot_util.h \
              data_plane_plot.h
