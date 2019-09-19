## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_pressure_levels

test_pressure_levels_SOURCES = test_pressure_levels.cc
test_pressure_levels_CPPFLAGS = ${MET_CPPFLAGS}
test_pressure_levels_LDFLAGS = -L. ${MET_LDFLAGS}
test_pressure_levels_LDADD = -lvx_tc_util \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_log \
	-lnetcdf_c++4
