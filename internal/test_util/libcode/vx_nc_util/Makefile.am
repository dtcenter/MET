## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

if ENABLE_PYTHON
LDFLAGS += -lvx_python3_utils
endif

# Test programs

noinst_PROGRAMS = test_pressure_levels

test_pressure_levels_SOURCES = test_pressure_levels.cc
test_pressure_levels_CPPFLAGS = ${MET_CPPFLAGS}
test_pressure_levels_LDFLAGS = -L. ${MET_LDFLAGS}
test_pressure_levels_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_shapedata \
	-lvx_util \
	$(PYTHON_LIBS) \
	-lvx_statistics \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_geodesy \
	-lvx_config \
	-lvx_gsl_prob \
	-lvx_cal \
	-lvx_nav \
	-lvx_util_math \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lvx_util

