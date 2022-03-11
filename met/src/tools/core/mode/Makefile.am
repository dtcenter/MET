## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = mode
mode_SOURCES = mode_usage.cc \
	mode_frontend.cc \
	mode_multivar.cc \
	mode_singlevar.cc \
	multivar_frontend.cc \
	mode.cc \
	combine_boolplanes.cc \
	objects_from_netcdf.cc \
	mode_ps_file.cc \
	plot_engine.cc \
	page_1.cc \
	fcst_enlarge_page.cc \
	obs_enlarge_page.cc \
	cluster_page.cc \
	overlap_page.cc \
	mode_exec.cc
mode_CPPFLAGS = ${MET_CPPFLAGS}
mode_LDFLAGS = ${MET_LDFLAGS}
mode_LDADD = -lvx_pxm \
	-lvx_plot_util \
	-lvx_nav \
	-lvx_render \
	-lvx_ps \
	-lvx_afm \
	-lvx_stat_out \
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
	-lvx_statistics \
	-lvx_data2d \
	-lvx_nc_util \
	-lvx_regrid \
	-lvx_grid \
	-lvx_config \
	-lvx_bool_calc \
	-lvx_cal \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz

EXTRA_DIST = mode_exec.h \
	mode_ps_file.h \
	mode_ps_table_defs.h \
	mode_usage.h \
	combine_boolplanes.h \
	objects_from_netcdf.h