## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_grid_area

test_grid_area_SOURCES = test_grid_area.cc
test_grid_area_CPPFLAGS = ${MET_CPPFLAGS}
test_grid_area_LDFLAGS = -L. ${MET_LDFLAGS}
test_grid_area_LDADD = -lvx_grid \
	-lvx_nav \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lz \
	-lm
