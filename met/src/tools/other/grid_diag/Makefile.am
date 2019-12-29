## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = grid_diag
grid_diag_SOURCES = grid_diag.cc \
	grid_diag_conf_info.cc
grid_diag_CPPFLAGS = ${MET_CPPFLAGS}
grid_diag_LDFLAGS = ${MET_LDFLAGS}
grid_diag_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_analysis_util \
	-lvx_series_data \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lvx_gsl_prob \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = grid_diag.h \
	grid_diag_conf_info.h
