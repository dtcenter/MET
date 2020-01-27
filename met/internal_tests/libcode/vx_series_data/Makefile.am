## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_series_data

test_series_data_SOURCES = test_series_data.cc
test_series_data_CPPFLAGS = ${MET_CPPFLAGS}
test_series_data_LDFLAGS = -L. ${MET_LDFLAGS}
test_series_data_LDADD = -lvx_config \
	-lvx_series_data \
	-lvx_nc_util \
	-lvx_log \
	-lvx_data2d \
	-lvx_grid  \
	-lvx_util \
	-lvx_cal \
	-lvx_math \
	-lvx_log \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_math \
	-lvx_log \
	-lvx_cal \
	-lvx_util \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
