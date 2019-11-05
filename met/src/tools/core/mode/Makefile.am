## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The library

EXTRA_DIST = mode_exec.h \
	mode_ps_file.h \
	mode_ps_table_defs.h

noinst_LIBRARIES=libmode.a
libmode_a_SOURCES = \
	mode_ps_file.cc \
	plot_engine.cc \
	page_1.cc \
	fcst_enlarge_page.cc \
	obs_enlarge_page.cc \
	cluster_page.cc \
	overlap_page.cc \
	mode_exec.cc

libmode_a_CPPFLAGS = ${MET_CPPFLAGS}
libmode_LDFLAGS = ${MET_LDFLAGS}
libmode_LDADD = -lvx_pxm \
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
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas -lz
