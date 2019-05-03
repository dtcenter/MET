## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_map_region

test_map_region_SOURCES = test_map_region.cc
test_map_region_CPPFLAGS = ${MET_CPPFLAGS}
test_map_region_LDFLAGS = -L. ${MET_LDFLAGS}
test_map_region_LDADD = -lvx_plot_util \
	-lvx_util \
	-lvx_log \
	-lm

