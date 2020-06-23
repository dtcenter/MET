## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# Test programs

noinst_PROGRAMS = test_is_grib \
	test_factory

test_is_grib_SOURCES = test_is_grib.cc
test_is_grib_CPPFLAGS = ${MET_CPPFLAGS}
test_is_grib_LDFLAGS = -L. ${MET_LDFLAGS}
test_is_grib_LDADD = -lvx_data2d_factory \
	-lvx_cal \
	-lvx_config \
	-lvx_util \
	-lvx_log

test_factory_SOURCES = test_factory.cc
test_factory_CPPFLAGS = ${MET_CPPFLAGS}
test_factory_LDFLAGS = -L. ${MET_LDFLAGS}
test_factory_LDADD = -lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
