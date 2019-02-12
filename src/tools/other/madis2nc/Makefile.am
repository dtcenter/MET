## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = madis2nc
madis2nc_SOURCES = madis2nc.cc madis2nc_conf_info.cc
madis2nc_CPPFLAGS = ${MET_CPPFLAGS}
madis2nc_LDFLAGS = ${MET_LDFLAGS}
madis2nc_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	$(PYTHON_LIBS) \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_obs \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lvx_summary \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = madis2nc.h \
	madis2nc_conf_info.h
