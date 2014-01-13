## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = tc_dland
tc_dland_SOURCES = tc_dland.cc \
	aland.f
tc_dland_CPPFLAGS = -I${includedir}
tc_dland_LDFLAGS = -L${libdir}
tc_dland_LDADD = -lvx_tc_util \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm \
	-lnetcdf_c++ -lnetcdf \
	-lgfortran
