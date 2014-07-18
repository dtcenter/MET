## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = tc_dland
tc_dland_SOURCES = tc_dland.cc \
	tc_poly_array.cc
tc_dland_CPPFLAGS = ${MET_CPPFLAGS}
tc_dland_LDFLAGS = ${MET_LDFLAGS}
tc_dland_LDADD = -lvx_tc_util \
	-lvx_nc_util \
	-lvx_grid \
        -lvx_nav \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm \
	-lnetcdf_c++ -lnetcdf \
	$(FLIBS)
