## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = tc_dland
tc_dland_SOURCES = tc_dland.cc \
	tc_poly.cc tc_poly.h
tc_dland_CPPFLAGS = ${MET_CPPFLAGS}
tc_dland_LDFLAGS = ${MET_LDFLAGS}
tc_dland_LDADD = -lvx_tc_util \
	$(UGRID_MET_LIBS) \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_gnomon \
	-lvx_nav \
	-lvx_log \
	-lvx_util_math \
	-lvx_util \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_math \
	$(UGRID_DEP_LIBS) \
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas \
	$(FLIBS)
