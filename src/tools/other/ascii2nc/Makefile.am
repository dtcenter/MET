## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

bin_PROGRAMS    = ascii2nc
ascii2nc_SOURCES = ascii2nc.cc \
	ascii2nc_conf_info.cc \
	file_handler.cc \
	little_r_handler.cc \
	met_handler.cc \
	observation.cc \
	summary_calc.cc \
	summary_calc_max.cc \
	summary_calc_mean.cc \
	summary_calc_median.cc \
	summary_calc_min.cc \
	summary_calc_percentile.cc \
	summary_calc_range.cc \
	summary_calc_stdev.cc \
	summary_key.cc \
	surfrad_handler.cc \
	time_summary_interval.cc
ascii2nc_CPPFLAGS = -I${includedir}
ascii2nc_LDFLAGS = -L${libdir}
ascii2nc_LDADD = -lvx_config \
	-lvx_nc_util \
	-lvx_grid \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++ -lnetcdf

EXTRA_DIST = ascii2nc_conf_info.h \
	file_handler.h \
	little_r_handler.h \
	met_handler.h \
	observation.h \
	summary_calc.h \
	summary_calc_max.h \
	summary_calc_mean.h \
	summary_calc_median.h \
	summary_calc_min.h \
	summary_calc_percentile.h \
	summary_calc_range.h \
	summary_calc_stdev.h \
	summary_key.h \
	surfrad_handler.h \
	time_summary_interval.h