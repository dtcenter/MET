## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS    = tc_diag
tc_diag_SOURCES = tc_diag.cc \
	python_tc_diag.cc \
	tc_diag_conf_info.cc
tc_diag_CPPFLAGS = ${MET_CPPFLAGS}
tc_diag_LDFLAGS = ${MET_LDFLAGS}
tc_diag_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_analysis_util \
	-lvx_series_data \
	-lvx_tc_util \
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
	-lm -lproj -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas \
	$(FLIBS)

EXTRA_DIST = tc_diag_conf_info.h \
	python_tc_diag.h \
	tc_diag.h
