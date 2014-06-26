## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = tc_stat
tc_stat_SOURCES = tc_stat.cc \
	tc_stat_conf_info.cc \
	tc_stat_files.cc \
	tc_stat_job.cc
tc_stat_CPPFLAGS = -I${includedir}
tc_stat_LDFLAGS = -L${libdir}
tc_stat_LDADD = -lvx_config \
	-lvx_data2d_nc_met \
	-lvx_analysis_util \
	-lvx_tc_util \
	-lvx_nc_util \
	-lvx_statistics \
	-lvx_gsl_prob \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm \
	-lnetcdf_c++ -lnetcdf -lgsl -lgslcblas

EXTRA_DIST = tc_stat_conf_info.h \
	tc_stat_files.h \
	tc_stat.h \
	tc_stat_job.h
