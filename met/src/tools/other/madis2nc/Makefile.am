## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = madis2nc
madis2nc_SOURCES = madis2nc.cc
madis2nc_CPPFLAGS = -I${includedir}
madis2nc_LDFLAGS = -L${libdir}
madis2nc_LDADD = -lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = madis2nc.h