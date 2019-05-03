## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = ascii2nc
ascii2nc_SOURCES = ascii2nc.cc \
	ascii2nc_conf_info.cc ascii2nc_conf_info.h \
	file_handler.cc file_handler.h \
	little_r_handler.cc little_r_handler.h \
	met_handler.cc met_handler.h \
	observation.cc observation.h \
	summary_calc.cc summary_calc.h \
	summary_calc_max.cc summary_calc_max.h \
	summary_calc_mean.cc summary_calc_mean.h \
	summary_calc_median.cc summary_calc_median.h \
	summary_calc_min.cc summary_calc_min.h \
	summary_calc_percentile.cc summary_calc_percentile.h \
	summary_calc_range.cc summary_calc_range.h \
	summary_calc_stdev.cc summary_calc_stdev.h \
	summary_key.cc summary_key.h \
	surfrad_handler.cc surfrad_handler.h \
	time_summary_interval.cc time_summary_interval.h \
	wwsis_handler.cc wwsis_handler.h \
	aeronet_handler.cc aeronet_handler.h
ascii2nc_CPPFLAGS = ${MET_CPPFLAGS}
ascii2nc_LDFLAGS = ${MET_LDFLAGS}
ascii2nc_LDADD = -lvx_shapedata \
	-lvx_analysis_util \
	-lvx_data2d_factory \
	-lvx_data2d_grib $(GRIB2_LIBS) \
	-lvx_data2d_nc_met \
	-lvx_data2d_nc_pinterp \
	-lvx_data2d_nccf \
	-lvx_nc_util \
	-lvx_data2d \
	-lvx_render \
	-lvx_plot_util \
	-lvx_pxm \
	-lvx_color \
	-lvx_ps \
	-lvx_afm \
	-lvx_pb_util \
	-lvx_nav \
	-lvx_gnomon \
	-lvx_stat_out \
	-lvx_statistics \
	-lvx_gsl_prob \
	-lvx_grid \
	-lvx_config \
	-lvx_util \
	-lvx_math \
	-lvx_cal \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas
