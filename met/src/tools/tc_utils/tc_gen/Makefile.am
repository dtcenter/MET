## @start 1
## Makefile.am -- Process this file with automake to produce Makefile.in
## @end 1

MAINTAINERCLEANFILES	= Makefile.in

# Include the project definitions

include ${top_srcdir}/Make-include

# The program

bin_PROGRAMS   = tc_gen
tc_gen_SOURCES = tc_gen.cc \
	tc_gen_conf_info.cc
tc_gen_CPPFLAGS = ${MET_CPPFLAGS}
tc_gen_LDFLAGS = ${MET_LDFLAGS}
tc_gen_LDADD = -lvx_stat_out \
	-lvx_statistics \
	-lvx_shapedata \
	-lvx_gsl_prob \
	-lvx_analysis_util \
	-lvx_tc_util \
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
	-lvx_pb_util \
	-lvx_cal \
	-lvx_nav \
	-lvx_util \
	-lvx_math \
	-lvx_color \
	-lvx_log \
	-lm -lnetcdf_c++4 -lnetcdf -lgsl -lgslcblas \
	$(FLIBS)

EXTRA_DIST = tc_gen_conf_info.h \
	tc_gen.h
