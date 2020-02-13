## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

OPT_PYTHON_SOURCES =

if ENABLE_PYTHON
LDFLAGS += -lvx_python3_utils
endif

# The program

bin_PROGRAMS    = stat_analysis
stat_analysis_SOURCES = stat_analysis.cc \
	aggr_stat_line.cc \
	parse_stat_line.cc \
	stat_analysis_job.cc $(OPT_PYTHON_SOURCES)
stat_analysis_CPPFLAGS = ${MET_CPPFLAGS}
stat_analysis_LDFLAGS = ${MET_LDFLAGS}
stat_analysis_LDADD = -lvx_stat_out \
	-lvx_analysis_util \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
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
	-lvx_time_series \
	-lvx_pb_util \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lvx_summary \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = stat_analysis.h \
	parse_stat_line.h \
	aggr_stat_line.h \
	stat_analysis_job.h
