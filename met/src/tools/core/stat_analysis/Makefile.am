## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = stat_analysis
stat_analysis_SOURCES = stat_analysis.cc \
	aggr_stat_line.cc \
	parse_stat_line.cc \
	stat_analysis_job.cc
stat_analysis_CPPFLAGS = ${MET_CPPFLAGS}
stat_analysis_LDFLAGS = ${MET_LDFLAGS}
stat_analysis_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_nc_met \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_config \
	-lvx_time_series \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lnetcdf_c++4 \
	-lm -lnetcdf_c++ -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = stat_analysis.h \
	parse_stat_line.h \
	aggr_stat_line.h \
	stat_analysis_job.h
